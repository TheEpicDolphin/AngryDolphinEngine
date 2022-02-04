
#include "opengl_renderer.h"

#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "material_manager.h"
#include "mesh_manager.h"
#include "rendering_pipeline_manager.h"

static void GLFWErrorCallback(int error, const char* description)
{
	fputs(description, stderr);
}

static void DestroyWindow(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void OpenGLRenderer::Initialize(int width, int height) 
{
    
	glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window_ = glfwCreateWindow(width, height, "Test GLFW Window", NULL, NULL);

    if (!window_)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void OpenGLRenderer::PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) {
	// This will avoid having any lags during rendering.
	pipeline_config_map_[pipeline->GetInstanceID()] = CreatePipelineConfiguration(pipeline);
}

bool OpenGLRenderer::RenderFrame(const std::vector<CameraParams>& cameras, const std::vector<RenderableObject>& renderable_objects) {
	// Only support one camera for now.
	CameraParams cam_params = cameras[0];

    if (!glfwWindowShouldClose(window_)) {
        glfwSwapBuffers(window_);
        glfwPollEvents();

		int* window_width;
		int* window_height;
		glfwGetFramebufferSize(window_, window_width, window_height);
		glViewport(0, 0, *window_width, *window_height);
		glClear(GL_COLOR_BUFFER_BIT);

		const glm::mat4 view_matrix = cam_params.world_transform;
		glm::mat4 projection_matrix;
		if (cam_params.is_orthographic) {
			// Orthographic projection matrix
			projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);
		}
		else {
			// Perspective projection matrix
			// Projection matrix : 45° Field of View, width:height ratio, display range : 0.1 unit (z near) <-> 100 units (z far)
			projection_matrix = glm::perspective(
				glm::radians(cam_params.vertical_fov),
				cam_params.aspect_ratio, 
				cam_params.near_clip_plane_z, 
				cam_params.far_clip_plane_z
			);
		}

		std::map<RenderableObjectBatchKey, RenderableObjectBatch> sorted_renderable_batches;
		for (RenderableObject renderable_object : renderable_objects) {
			// TODO: perform culling

			const std::shared_ptr<RenderingPipeline>& pipeline = renderable_object.mesh->GetPipeline();

			if (pipeline_config_map_.find(pipeline->GetInstanceID()) == pipeline_config_map_.end()) {
				pipeline_config_map_[pipeline->GetInstanceID()] = CreatePipelineConfiguration(pipeline);
				pipeline.lifecycle_events_listener = this;
			}

			if (mesh_config_map_.find(renderable_object.mesh->GetInstanceID()) == mesh_config_map_.end()) {
				mesh_config_map_[renderable_object.mesh->GetInstanceID()] = CreateMeshConfiguration(renderable_object.mesh);
				renderable_object.mesh->lifecycle_events_listener = this;
			}
			
			const RenderableObjectBatchKey batch_key = {
				pipeline->GetInstanceID(),
				renderable_object.mesh->GetInstanceID(),
				renderable_object.material->GetInstanceID()
			};

			std::map<RenderableObjectBatchKey, RenderableObjectBatch>::iterator it = sorted_renderable_batches.find(batch_key);
			if (it == sorted_renderable_batches.end()) {
				const RenderableObjectBatch batch = { renderable_object.mesh, renderable_object.material, { {renderable_object.model_matrix, renderable_object.bones} } };
				sorted_renderable_batches.insert({ batch_key, batch });
			}
			else {
				it->second.instances.push_back({ renderable_object.model_matrix, renderable_object.bones });
			}	
		}

		const glm::mat4 vp = view_matrix * projection_matrix;
		RenderableObjectBatchKey previous_batch_key = { 0, 0, 0 };
		GLint mvp_location;
		GLint bones_location;
		for (std::map<RenderableObjectBatchKey, RenderableObjectBatch>::iterator it = sorted_renderable_batches.begin();
			it != sorted_renderable_batches.end();
			it++) {
			RenderableObjectBatchKey current_batch_key = it->first;
			RenderableObjectBatch batch = it->second;
			if (current_batch_key.pipeline_id != previous_batch_key.pipeline_id) {
				// Switch rendering pipeline configuration
				const PipelineConfiguration pipeline_config = pipeline_config_map_[current_batch_key.pipeline_id];
				glUseProgram(pipeline_config.program_id);
				// Find location of mvp matrix. This uniform is treated differently from the ones in Materials
				mvp_location = glGetUniformLocation(pipeline_config.program_id, "mvp");
				// Find location of bones array, if it exists. This uniform is treated differently from the ones in Materials
				bones_location = glGetUniformLocation(pipeline_config.program_id, "bones");
			}
			if (current_batch_key.mesh_id != previous_batch_key.mesh_id) {
				// Switch mesh configuration
				const MeshConfiguration mesh_config = mesh_config_map_[current_batch_key.mesh_id];
				glBindVertexArray(mesh_config.vao);
			}
			if (current_batch_key.material_id != previous_batch_key.material_id) {
				// Switch material configuration
				// Iterate material uniforms and set corresponding uniforms in shaders
				for (const UniformValue& uniform_value : batch.material->UniformValues()) {
					const UniformInfo& uniform_info = batch.mesh->GetPipeline()->UniformInfoAtIndex(uniform_value.uniform_index);
					shader::opengl::SetUniform(uniform_info.type, uniform_info.location, uniform_info.array_length, uniform_value.data.data());
				}
			}

			// Iterate over instances of mesh and draw each after transforming (and optionally setting bones).
			for (RenderableObjectInstance& renderable_object_instance : batch.instances) {
				const glm::mat4 mvp = renderable_object_instance.model_transform * vp;
				// Set MVP matrix in shader.
				glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

				if (bones_location) {
					// Set bones array in shader.
					glUniformMatrix4fv(
						bones_location,
						renderable_object_instance.bone_transforms.size(),
						GL_FALSE,
						reinterpret_cast<const GLfloat*>(renderable_object_instance.bone_transforms.data())
					);
				}

				// Draw
				glDrawArrays(GL_TRIANGLES, 0, batch.mesh->VertexCount());
			}

			previous_batch_key = current_batch_key;
		}

		glBindVertexArray(0);
		glUseProgram(0);

		// TODO: Keep track of areas in viewport that haven't been rendered yet. 
		// Once the entire viewport has been rendered, stop the enumeration.
    }
    else {
        return false;
    }

    return true;
}

void OpenGLRenderer::Cleanup() {
    DestroyWindow(window_);

	// TODO: Delete VAOs and VBOs
}

GLenum GLShaderTypeForStageType(ShaderStageType type) 
{
	switch (type)
	{
	case ShaderStageTypeVertex:
		return GL_VERTEX_SHADER;
	case ShaderStageTypeGeometry:
		return GL_GEOMETRY_SHADER;
	case ShaderStageTypeFragment:
		return GL_FRAGMENT_SHADER;
	case ShaderStageTypeCompute:
		return GL_COMPUTE_SHADER;
	}
}

std::string NameForStageType(ShaderStageType type)
{
	switch (type)
	{
	case ShaderStageTypeVertex:
		return "Vertex Shader Stage";
	case ShaderStageTypeGeometry:
		return "Geometry Shader Stage";
	case ShaderStageTypeFragment:
		return "Fragment Shader Stage";
	case ShaderStageTypeCompute:
		return "Compute Shader Stage";
	}
}

OpenGLRenderer::PipelineConfiguration OpenGLRenderer::CreatePipelineConfiguration(const std::shared_ptr<RenderingPipeline>& pipeline)
{
	const std::vector<Shader> shader_stages = pipeline->ShaderStages();
	GLuint program_id = glCreateProgram();
	std::vector<GLuint> shader_ids;
	shader_ids.reserve(shader_stages.size());
	GLint result = GL_FALSE;
	int info_log_length;
	for (Shader shader : shader_stages) {
		// Create the shader
		GLuint shader_id = glCreateShader(GLShaderTypeForStageType(shader.type));

		// Compile shader
		printf("Compiling %s...\n", NameForStageType(shader.type));
		char const* shader_source_ptr = shader.code.data();
		glShaderSource(shader_id, 1, &shader_source_ptr, NULL);
		glCompileShader(shader_id);

		// Check vertex shader
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			std::vector<char> shader_error_message(info_log_length + 1);
			glGetShaderInfoLog(shader_id, info_log_length, NULL, &shader_error_message[0]);
			printf("%s\n", &shader_error_message[0]);
		}

		shader_ids.push_back(shader_id);
		glAttachShader(program_id, shader_id);
	}

	// Link the program
	printf("Linking the program...\n");
	glLinkProgram(program_id);

	// Check the program
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> program_error_message(info_log_length + 1);
		glGetProgramInfoLog(program_id, info_log_length, NULL, &program_error_message[0]);
		printf("%s\n", &program_error_message[0]);
	}

	for (GLuint shader_id : shader_ids) {
		glDetachShader(program_id, shader_id);
		glDeleteShader(shader_id);
	}

	return { program_id };
}

OpenGLRenderer::MeshConfiguration OpenGLRenderer::CreateMeshConfiguration(Mesh* mesh) {
	MeshConfiguration mesh_config = { mesh->IsStatic() ? MeshDataUsageTypeStatic : MeshDataUsageTypeDynamic, 0, 0, 0 };
	mesh_config.SetupVertexAttributeBuffers(mesh);
	return mesh_config;
}

void OpenGLRenderer::MeshConfiguration::SetupVertexAttributeBuffers(Mesh* mesh)
{
	switch (data_usage_type) {
	case MeshDataUsageTypeStatic:
		// TODO
		break;
	case MeshDataUsageTypeDynamic:
		const std::shared_ptr<RenderingPipeline>& pipeline = mesh->GetPipeline();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		for (const VertexAttributeBuffer& va_buffer : mesh->GetVertexAttributeBuffers()) {
			const VertexAttributeInfo& vertex_attribute = pipeline->VertexAttributeInfoAtIndex(va_buffer.attribute_index);
			switch (vertex_attribute.category)
			{
			case VertexAttributeUsageCategoryPosition:
				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, mesh->VertexCount(), va_buffer.data.data(), GL_DYNAMIC_DRAW);
				break;
			case VertexAttributeUsageCategoryNormal:
				//glGenBuffers(1, &nbo);
				//glBindBuffer(GL_ARRAY_BUFFER, nbo);
				//glBufferData(GL_ARRAY_BUFFER, mesh->VertexCount(), va_buffer.data.data(), GL_DYNAMIC_DRAW);
				break;
			case VertexAttributeUsageCategoryTextureCoordinates:
				//glGenBuffers(1, &tbo);
				//glBindBuffer(GL_ARRAY_BUFFER, tbo);
				//glBufferData(GL_ARRAY_BUFFER, mesh->VertexCount(), va_buffer.data.data(), GL_DYNAMIC_DRAW);
				break;
			case VertexAttributeUsageCategoryBoneWeights:
				//glGenBuffers(1, &bwbo);
				//glBindBuffer(GL_ARRAY_BUFFER, tbo);
				//glBufferData(GL_ARRAY_BUFFER, mesh->VertexCount(), va_buffer.data.data(), GL_DYNAMIC_DRAW);
				break;
			case VertexAttributeUsageCategoryBoneIndices:

				break;
			case VertexAttributeUsageCategoryCustom:
				//glGenBuffers(1, &custom_bo[vertex_attribute.name]);
				//glBindBuffer(GL_ARRAY_BUFFER, custom_bo[vertex_attribute.name]);
				break;
			}

			glEnableVertexAttribArray(vertex_attribute.location);
			glVertexAttribPointer(
				vertex_attribute.location,		// The shader's location for vertex attribute.
				vertex_attribute.dimension,		// number of components
				vertex_attribute.format,		// type
				GL_FALSE,						// normalized?
				0,								// stride
				(void*)0						// array buffer offset
			);
			glDisableVertexAttribArray(vertex_attribute.location);
		}
		glBindVertexArray(0);
		break;
	}
}
