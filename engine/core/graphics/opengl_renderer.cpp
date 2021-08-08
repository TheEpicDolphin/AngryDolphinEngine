
#include "opengl_renderer.h"

#include <GL/glew.h>
#include <string>

#include <core/utils/file_helpers.h>

static void DestroyWindow(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void OpenGLRenderer::Initialize(int width, int height) 
{
	material_manager.delegate = this;
	pipeline_manager.delegate = this;
	material_manager_.LoadMaterialSpecs();
	pipeline_manager_.LoadPipelineSpecs();

    glfwSetErrorCallback(error_callback);

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

void OpenGLRenderer::AddRenderableObject(UID id, RenderableObjectInfo info)
{
	if (renderable_object_map_.find(id) != renderable_object_map_.end()) {
		const RenderableID& renderable_id = renderable_object_map_[id];

		// TODO: Map mesh batches to material batches
		const PipelineID pipeline_id = info.material->GetPipelineID();
		const PipelineBatch& pipeline_batch = pipeline_batch_map_[pipeline_id];
		const MeshID mesh_id = info.mesh->GetInstanceID();
		MeshBatch& mesh_batch = mesh_batch_map_[mesh_id];
		
		MaterialBatch& material_batch = material_batch_map_[info.material->GetInstanceID()];
		if (std::find(material_batch.mesh_ids.begin(), material_batch.mesh_ids.end(), mesh_id) != material_batch.mesh_ids.end()) {
			// mesh has already been mapped to this material
			return;
		}

		material_batch.mesh_ids.push_back(mesh_id);

		glBindVertexArray(mesh_batch.vao);
		const std::vector<VertexAttributeInfo>& vertex_attributes = info.material->GetPipeline()->VertexAttributes();
		for (VertexAttributeInfo vertex_attribute : vertex_attributes) {
			mesh_batch.SetVertexAttributes(vertex_attribute);
			/*
			glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);

			GLuint attrib_location = AttributeLocationForVertexAttributeType(vertex_attribute_type);
			glEnableVertexAttribArray(attrib_location);
			glVertexAttribPointer(
				attrib_location,												// The shader's location for vertex attribute.
				AttributeSizeForVertexAttributeType(vertex_attribute_type),		// size
				GLDataTypeForVertexAttributeType(vertex_attribute_type),		// type
				GL_FALSE,														// normalized?
				0,																// stride
				(void*)0														// array buffer offset
			);
			glDisableVertexAttribArray(attrib_location);
			*/
		}
	}
	renderable_object_map_[id] = { info.material->GetInstanceID(), info.mesh->GetInstanceID() };
}

void RemoveRenderableObject(UID id)
{
	renderable_object_map_.erase(id);
}

bool OpenGLRenderer::RenderFrame() {
    if (!glfwWindowShouldClose(window_)) {
        glfwSwapBuffers(window_);
        glfwPollEvents();

		int* width;
		int* height;
		glfwGetFramebufferSize(window_, width, height);
		glViewport(0, 0, *width, *height);
		glClear(GL_COLOR_BUFFER_BIT);

		if (!camera.enabled) {
			// Camera is disabled, don't render
			return;
		}

		const glm::mat4 view_matrix = transform.matrix;
		glm::mat4 projection_matrix;
		if (camera.is_orthographic) {
			// Orthographic projection matrix
			projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);
		}
		else {
			// Perspective projection matrix
			// Projection matrix : 45° Field of View, width:height ratio, display range : 0.1 unit (z near) <-> 100 units (z far)
			projection_matrix = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
		}
		
		const glm::mat4 vp = view_matrix * projection_matrix;
		for (auto& p_batch_iter : pipeline_batch_map_) {
			PipelineBatch& pipeline_batch = p_batch_iter.second;
			glUseProgram(pipeline_batch.program_id);
			for (auto& mat_batch_iter : material_batch_map_) {
				for (auto& mesh_batch_iter : mesh_batch_map_) {
					MeshBatch& mesh_batch = mesh_batch_iter.second;
					glBindVertexArray(mesh_batch.vao);
					GLint mvp_location = glGetUniformLocation(pipeline_batch.program_id, "mvp");
					for (glm::mat4& model_matrix : mesh_batch.model_matrix_map) {
						const glm::mat4 mvp = model_matrix * vp;
						// Insert MVP matrix into shader
						glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
						// Draw
						glDrawArrays(GL_TRIANGLES, 0, mesh_batch.vertex_count);
					}
				}
			}
		}

		glBindVertexArray(0);
		glUseProgram(0);

		// TODO: Do above inside window context
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

std::shared_ptr<RenderingPipeline> OpenGLRenderer::CreateRenderingPipeline(RenderingPipelineInfo info)
{
	const std::shared_ptr<RenderingPipeline> pipeline = pipeline_manager_.CreatePipeline(info);
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

	pipeline_batch_map_[pipeline->GetInstanceID()] = { program_id, {} };	
	return pipeline;
}

std::shared_ptr<Material> OpenGLRenderer::CreateMaterial(MaterialInfo info)
{
	const std::shared_ptr<Material> material = material_manager_.CreateMaterial(info);
	const PipelineID pipeline_Id = info.rendering_pipeline->GetInstanceID();
	std::unordered_map<PipelineID, PipelineBatch>::iterator iter = pipeline_batch_map_.find(pipeline_Id);
	if (iter != pipeline_batch_map_.end()) {
		PipelineBatch* pipeline_batch = &iter->second;
		pipeline_batch->material_ids.push_back(material->GetInstanceID());
		material_batch_map_[material->GetInstanceID()] = { {} };
	}
	else {
		// This pipeline should have been registered in this class already when it was created.
	}
}

std::shared_ptr<Mesh> OpenGLRenderer::CreateMesh(MeshInfo info) 
{
	const std::shared_ptr<Mesh> mesh = mesh_manager_.CreateMesh(info);
	MeshBatch mb = { mesh->GetVertices().size(), 0, 0, 0, {} };
	std::pair<std::unordered_map<MeshID, MeshBatch>::iterator, bool> result = mesh_batch_map_.insert(std::make_pair(mesh->GetInstanceID(), mb));
	MeshBatch& mesh_batch = result.first->second;

	glGenVertexArrays(1, &mesh_batch.vao);
	glBindVertexArray(mesh_batch.vao);

	glGenBuffers(1, &mesh_batch.vbo);
	//glGenBuffers(1, &mesh_batch.ibo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_batch.ibo);
	glBufferData(GL_ARRAY_BUFFER, 3, mesh->GetVertices().data(), GL_STATIC_DRAW);

	return mesh;
}

