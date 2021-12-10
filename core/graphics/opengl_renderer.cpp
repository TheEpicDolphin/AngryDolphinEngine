
#include "opengl_renderer.h"

#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <config/generated/config.h>
#include <core/utils/file_helpers.h>

static void DestroyWindow(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void OpenGLRenderer::Initialize(int width, int height) 
{
	material_manager_.delegate = this;
	pipeline_manager_.delegate = this;
	LoadRenderingAssets();

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

bool OpenGLRenderer::RenderFrame(const std::vector<RenderableObject>& renderable_objects) {
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

		const glm::mat4 view_matrix = camera.transform.matrix;
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

		std::map<RenderableObjectKey, std::vector<RenderableObjectInstance>> sorted_renderable_objects;
		for (RenderableObject renderable_object : renderable_objects) {
			// TODO: perform culling
			
			const RenderableObjectKey key = {
				renderable_object.mesh->GetPipeline()->GetInstanceID(), 
				renderable_object.mesh->GetInstanceID(),
				renderable_object.mesh->GetMaterial()->GetInstanceID()
			};

			if () {
				sorted_renderable_objects.insert({ key, { {renderable_object.mesh, renderable_object.model_matrix, renderable_object.bones } } });
			}
			else {
				sorted_renderable_objects[key].push_back({ renderable_object.mesh, renderable_object.model_matrix, renderable_object.bones });
			}	
		}

		const glm::mat4 vp = view_matrix * projection_matrix;
		RenderableObjectKey previous_key = sorted_renderable_objects.begin()->first;
		GLint mvp_location;
		GLint bones_location;
		for (std::map<RenderableObjectKey, std::vector<RenderableObjectInstance>>::iterator it = sorted_renderable_objects.begin(); 
			it != sorted_renderable_objects.end(); 
			it++) {
			RenderableObjectKey current_key = it->first;
			if (current_key.pipeline_id != previous_key.pipeline_id) {
				// Switch rendering pipeline configuration
				const PipelineConfiguration pipeline_config = pipeline_config_map_[current_key.pipeline_id];
				glUseProgram(pipeline_config.program_id);
				// Find location of mvp matrix. This uniform is treated differently from the ones in Materials
				mvp_location = glGetUniformLocation(pipeline_config.program_id, "mvp");
				// Find location of bones array, if it exists. This uniform is treated differently from the ones in Materials
				bones_location = glGetUniformLocation(pipeline_config.program_id, "bones");
			}
			if (current_key.mesh_id != previous_key.mesh_id) {
				// Switch mesh configuration
				const MeshConfiguration mesh_config = mesh_config_map_[current_key.mesh_id];
				glBindVertexArray(mesh_config.vao);

			}
			if (current_key.material_id != previous_key.material_id) {
				// Switch material configuration
				// Iterate material uniforms and set corresponding uniforms in shaders
				for (const UniformValue& uniform_value : material->UniformValues()) {
					const UniformInfo& uniform_info = pipeline->UniformInfoAtIndex(uniform_value.uniform_index);
					shader::opengl::SetUniform(uniform_info.type, uniform_info.location, uniform_info.array_length, uniform_value.data.data());
				}
			}

			// Iterate over instances of mesh and draw each after transforming (and optionally setting bones).
			for (RenderableObjectInstance& renderable_object_instance : mesh_batch->renderable_object_instances) {
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
				glDrawArrays(GL_TRIANGLES, 0, mesh_batch->mesh->VertexCount());
			}

			previous_key = current_key;
		}
		
		const glm::mat4 vp = view_matrix * projection_matrix;
		for (auto& p_batch_iter : pipeline_batch_map_) {
			PipelineBatch& pipeline_batch = p_batch_iter.second;
			glUseProgram(pipeline_batch.program_id);
			// Find location of mvp matrix. This uniform is treated differently from the ones in Materials
			GLint mvp_location = glGetUniformLocation(pipeline_batch.program_id, "mvp");
			// Find location of bones array, if it exists. This uniform is treated differently from the ones in Materials
			GLint bones_location = glGetUniformLocation(pipeline_batch.program_id, "bones");
			for (MeshID& mesh_id : pipeline_batch.mesh_ids) {
				IMeshBatch* mesh_batch = mesh_batch_map_[mesh_id];
				glBindVertexArray(mesh_batch->vao);

				// TODO: sort meshes by material id to reduce number of times setting material uniforms.

				const std::shared_ptr<RenderingPipeline>& pipeline = mesh_batch->mesh->GetPipeline();
				const std::shared_ptr<Material>& material = mesh_batch->mesh->GetMaterial();
				// Iterate material uniforms and set corresponding uniforms in shaders
				for (const UniformValue& uniform_value : material->UniformValues()) {
					const UniformInfo& uniform_info = pipeline->UniformInfoAtIndex(uniform_value.uniform_index);
					shader::opengl::SetUniform(uniform_info.type, uniform_info.location, uniform_info.array_length, uniform_value.data.data());
				}
				
				// Iterate over instances of mesh and draw each after transforming (and optionally setting bones).
				for (RenderableObjectInstance& renderable_object_instance : mesh_batch->renderable_object_instances) {
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
					glDrawArrays(GL_TRIANGLES, 0, mesh_batch->mesh->VertexCount());
				}
			}
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

	pipeline_config_map_[pipeline->GetInstanceID()] = { program_id };
	return pipeline;
}

std::unique_ptr<Material> OpenGLRenderer::CreateUniqueMaterial(MaterialInfo info)
{
	const std::unique_ptr<Material> material = material_manager_.CreateUniqueMaterial(info);
	return std::move(material);
}

std::shared_ptr<Material> OpenGLRenderer::CreateSharedMaterial(MaterialInfo info)
{
	const std::shared_ptr<Material> material = material_manager_.CreateSharedMaterial(info);
	return material;
}

std::unique_ptr<Mesh> OpenGLRenderer::CreateUniqueMesh(MeshInfo info)
{
	const std::unique_ptr<Mesh> mesh = mesh_manager_.CreateUniqueMesh(info);
	MeshConfiguration mesh_config;
	if (info.is_static) {
		// TODO: Create a static mesh batch. 
	}
	else {
		mesh_config = { MeshDataUsageTypeDynamic, 0, 0, 0 };
		mesh_config.SetupVertexAttributeBuffers(mesh.get());
	}

	mesh_config_map_[mesh->GetInstanceID()] = mesh_config;
	return mesh;
}

std::shared_ptr<Mesh> OpenGLRenderer::CreateSharedMesh(MeshInfo info) 
{
	const std::shared_ptr<Mesh> mesh = mesh_manager_.CreateSharedMesh(info);
	MeshConfiguration mesh_config;
	if (info.is_static) {
		// TODO: Create a static mesh batch. 
	}
	else {
		mesh_config = { MeshDataUsageTypeDynamic, 0, 0, 0};
		mesh_config.SetupVertexAttributeBuffers(mesh.get());
	}
	
	mesh_config_map_[mesh->GetInstanceID()] = mesh_config;
	return mesh;
}

void OpenGLRenderer::MeshConfiguration::SetupVertexAttributeBuffers(Mesh* mesh)
{
	switch (data_usage_type) {
	case MeshDataUsageTypeStream:
		break;
	case MeshDataUsageTypeStatic:
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

void OpenGLRenderer::LoadRenderingAssets() {
	// Recursively searches for .rp files in the project's resources folder.
	const std::vector<fs::path> rp_file_paths = file_helpers::AllFilePathsInDirectoryWithExtension(GAME_RESOURCES_DIRECTORY, ".rp");
	for (fs::path rp_file_path : rp_file_paths) {
		std::vector<char> source_code = file_helpers::ReadFileWithPath(rp_file_path);
		int asset_id = asset_filepath_hasher_.size();
		pipeline_manager_.CreatePipelineWithHash({ source_code }, asset_id);
		asset_filepath_hasher_[rp_file_path.u8string()] = asset_id;

		// Look for mesh spec files
		fs::path mesh_specs_folder("mesh_specs");
		fs::path mesh_spec_folder_path = rp_file_path.parent_path() / mesh_specs_folder;
		std::vector<fs::path> mesh_spec_filepaths = file_helpers::AllFilePathsInDirectoryWithExtension(mesh_spec_folder_path.u8string(), ".meshspec");
		for (fs::path mesh_spec_path : mesh_spec_filepaths) {
			std::vector<char> mesh_spec_contents = file_helpers::ReadFileWithPath(mesh_spec_path);
			int mesh_asset_id = asset_filepath_hasher_.size();
			mesh_manager_.CreateMeshWithHash({ source_code }, mesh_asset_id);
			asset_filepath_hasher_[mesh_spec_path.u8string()] = mesh_asset_id;
		}

		// Look for material spec files
		fs::path material_specs_folder("material_specs");
		fs::path material_specs_folder_path = rp_file_path.parent_path() / material_specs_folder;
		std::vector<fs::path> material_spec_filepaths = file_helpers::AllFilePathsInDirectoryWithExtension(material_specs_folder_path.u8string(), ".materialspec");
		for (fs::path material_spec_path : material_spec_filepaths) {
			std::vector<char> material_spec_contents = file_helpers::ReadFileWithPath(material_spec_path);
			int material_asset_id = asset_filepath_hasher_.size();
			material_manager_.CreateMaterialWithHash({ source_code }, material_asset_id);
			asset_filepath_hasher_[material_spec_path.u8string()] = material_asset_id;
		}
	}
}
