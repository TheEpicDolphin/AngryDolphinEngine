
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
			for (auto& mesh_batch_iter : mesh_batch_map_) {
				IMeshBatch *mesh_batch = mesh_batch_iter.second;
				glBindVertexArray(mesh_batch->vao);

				// TODO: Iterate material uniforms and set corresponding uniforms in shaders
				for () {
					glUniformMatrix4fv(uniform_location, 1, GL_FALSE, mesh_batch->mesh->material->GetUniformData(name));
				}

				GLint mvp_location = glGetUniformLocation(pipeline_batch.program_id, "mvp");
				for (glm::mat4& model_matrix : mesh_batch.model_matrix_map) {
					const glm::mat4 mvp = model_matrix * vp;
					// Insert MVP matrix into shader
					glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
					// Draw
					glDrawArrays(GL_TRIANGLES, 0, mesh_batch->mesh->VertexCount());
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
	return material;
}

std::shared_ptr<Mesh> OpenGLRenderer::CreateMesh(MeshInfo info) 
{
	const std::shared_ptr<Mesh> mesh = mesh_manager_.CreateMesh(info);
	IMeshBatch* mb;
	if (info.is_static) {
		// TODO: Create a static mesh batch. 
	}
	else {
		mb = new DynamicMeshBatch(mesh, 0, 0);
		mb->ConfigureBuffersForVertexAttributes();
	}
	mesh_batch_map_[mesh->GetInstanceID()] = mb;
	return mesh;
}

OpenGLRenderer::DynamicMeshBatch::DynamicMeshBatch(std::shared_ptr<Mesh> mesh, GLuint vao, GLuint vbo)
{
	this->mesh = mesh;
	this->vao = vao;
	this->vbo = vbo;
}

void OpenGLRenderer::DynamicMeshBatch::SetupVertexAttributeBuffers()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	for () {
		switch (vertex_attribute.category)
		{
		case VertexAttributeUsageCategoryPosition:
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->VertexCount(), mesh->GetVertexPositions().data(), GL_DYNAMIC_DRAW);
			break;
		case VertexAttributeUsageCategoryNormal:
			//glGenBuffers(1, &nbo);
			//glBindBuffer(GL_ARRAY_BUFFER, nbo);
			break;
		case VertexAttributeUsageCategoryTextureCoordinates:
			//glGenBuffers(1, &tbo);
			//glBindBuffer(GL_ARRAY_BUFFER, tbo);
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
}
