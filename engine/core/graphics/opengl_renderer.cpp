
#include "opengl_renderer.h"

#include <GL/glew.h>
#include <string>

#include <core/utils/file_helpers.h>

static MeshBatch CreateMeshBatch(GLuint& vao, Material& material, Mesh& mesh) 
{
	glBindVertexArray(vao);
	glUseProgram(material.ProgramID());

	MeshBatch mesh_batch = {
		0,
		0,
		mesh,
		{}
	};

	glGenBuffers(1, &mesh_batch.vbo);
	//glGenBuffers(1, &mesh_batch.ibo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_batch.ibo);
	glBufferData(GL_ARRAY_BUFFER, 3, mesh.GetVertices().data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(material.VertexAttribute());
	glVertexAttribPointer(
		material.VertexAttribute(), // The shader's attribute for vertex positions.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glDisableVertexAttribArray(material.VertexAttribute());
	return mesh_batch;
}

static void DestroyWindow() {
    glfwDestroyWindow(window_);
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
		const RenderableID& renderableID = renderable_object_map_[id];
		MaterialBatch* const material_batch = &material_batch_map_[renderableID.material_id];
		MeshBatch* const mesh_batch = &material_batch->mesh_batch_map[renderableID.mesh_id];
		if (info.material != material_batch->material) {
			// The material for this renderable object has changed.
			
			if (material_batch_map_.find(info.material->GetInstanceID()) != material_batch_map_.end()) {
				MaterialBatch* const existing_material_batch = &material_batch_map_[info.mesh->GetInstanceID()];

				if (existing_material_batch->mesh_batch_map.find(info.mesh->GetInstanceID()) != existing_material_batch->mesh_batch_map.end()) {
					MeshBatch* const existing_mesh_batch_2 = &existing_material_batch->mesh_batch_map[info.mesh->GetInstanceID()];
					existing_mesh_batch_2->model_matrix_map[id] = info.model_matrix;
				}
				else {
					// This mesh is being added to a renderable object for the first time.
					MeshBatch mesh_batch = CreateMeshBatch(material_batch->vao, *info.material.get(), *info.mesh.get());
					mesh_batch.model_matrix_map[id] = info.model_matrix;
					material_batch->mesh_batch_map[info.mesh->GetInstanceID()] = mesh_batch;
				}
			}
			else {
				// This material is being added to a renderable object for the first time.
				MaterialBatch material_batch = { 0 };
				material_batch.material = info.material;

				glGenVertexArrays(1, &material_batch.vao);
				MeshBatch mesh_batch = CreateMeshBatch(material_batch.vao, *info.material.get(), *info.mesh.get());
				mesh_batch.model_matrix_map[id] = info.model_matrix;

				material_batch.mesh_batch_map[info.mesh->GetInstanceID()] = mesh_batch;
				material_batch_map_[info.material->GetInstanceID()] = material_batch;
			}
			
			mesh_batch->model_matrix_map.erase(id);
			if (mesh_batch->model_matrix_map.size() == 0) {
				material_batch->mesh_batch_map.erase(renderableID.mesh_id);
				if (material_batch->mesh_batch_map.size() == 0) {
					material_batch_map_.erase(renderableID.material_id);
				}
			}

			return;
		}

		if (info.mesh != mesh_batch->mesh) {
			// The mesh for this renderable object has changed.

			if (material_batch->mesh_batch_map.find(info.mesh->GetInstanceID()) != material_batch->mesh_batch_map.end()) {
				MeshBatch *const existing_mesh_batch = &material_batch->mesh_batch_map[info.mesh->GetInstanceID()];
				existing_mesh_batch->model_matrix_map[id] = info.model_matrix;
			}
			else {
				// This mesh is being added to a renderable object for the first time.
				MeshBatch mesh_batch = CreateMeshBatch(material_batch->vao, *info.material.get(), *info.mesh.get());
				mesh_batch.model_matrix_map[id] = info.model_matrix;
				material_batch->mesh_batch_map[info.mesh->GetInstanceID()] = mesh_batch;
			}

			mesh_batch->model_matrix_map.erase(id);
			if (mesh_batch->model_matrix_map.size() == 0) {
				material_batch->mesh_batch_map.erase(renderableID.mesh_id);
			}

			return;
		}

		// Update the model matrix for this renderable object.
		mesh_batch->model_matrix_map[id] = info.model_matrix;
	}
	else {
		// This renderable object is being set for the first time.
		const InstanceID materialID = info.material->GetInstanceID();
		const InstanceID meshID = info.mesh->GetInstanceID();
		if (material_batch_map_.find(materialID) != material_batch_map_.end()) {
			MaterialBatch* const material_batch = &material_batch_map_[materialID];
			if (material_batch->mesh_batch_map.find(meshID) != material_batch->mesh_batch_map.end()) {
				MeshBatch* const mesh_batch = &material_batch->mesh_batch_map[meshID];
				mesh_batch->model_matrix_map[id] = info.model_matrix;
			}
			else {
				// This mesh is being added to a renderable object for the first time.
				MeshBatch mesh_batch = CreateMeshBatch(material_batch->vao, *info.material.get(), *info.mesh.get());
				mesh_batch.model_matrix_map[id] = info.model_matrix;
				material_batch->mesh_batch_map[meshID] = mesh_batch;
			}
		}
		else {
			// This material is being added to a renderable object for the first time.
			MaterialBatch material_batch = { 0 };
			material_batch.material = info.material;

			glGenVertexArrays(1, &material_batch.vao);
			MeshBatch mesh_batch = CreateMeshBatch(material_batch.vao, *info.material.get(), *info.mesh.get());
			mesh_batch.model_matrix_map[id] = info.model_matrix;

			material_batch.mesh_batch_map[meshID] = mesh_batch;
			material_batch_map_[materialID] = material_batch;
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

		for (auto& p_batch_iter : pipeline_batch_map_) 
		{
			PipelineBatch pipeline_batch = p_batch_iter.second;
			glUseProgram(pipeline_batch.program_id);


			MaterialBatch material_batch = it.second;
			glBindVertexArray(material_batch.vao);
			GLint mvp_location = glGetUniformLocation(pipeline_batch.program_id, "mvp");
		}

		for (auto& it : material_batch_map)
		{
			MaterialBatch material_batch = it.second;
			glBindVertexArray(material_batch.vao);
			GLint mvp_location = glGetUniformLocation(material_batch.material.ProgramID(), "mvp");
			for (auto& it : material_batch.mesh_batch_map) {
				MeshBatch mesh_batch = it.second;

				// set vertex attributes for mesh

				for () {

				}
				// get the location of attribute "position" in program <code>p</code>
				vertexLoc = glGetAttribLocation(p, "position");
				vertexLoc = glGetAttribLocation(p, "position");
				vertexLoc = glGetAttribLocation(p, "position");

				// bind buffer for positions and copy data into buffer
				// GL_ARRAY_BUFFER is the buffer type we use to feed attributes
				glBindBuffer(GL_ARRAY_BUFFER, buffer);

				// feed the buffer, and let OpenGL know that we don't plan to
				// change it (STATIC) and that it will be used for drawing (DRAW)
				glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

				// Enable the attribute at that location
				glEnableVertexAttribArray(vertexLoc);

				// Tell OpenGL what the array contains: 
				// it is a set of 4 floats for each vertex
				glVertexAttribPointer(vertexLoc, 4, GL_FLOAT, 0, 0, 0);

				for (glm::mat4& model_matrix : mesh_batch.model_matrices) {
					const glm::mat4 mvp = model_matrix * view_matrix * projection_matrix;
					// Insert MVP matrix into shader
					glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
					// Draw
					glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
				}
				glDeleteBuffers(1, &mesh_batch.vbo);
			}
			glDeleteVertexArrays(1, &material_batch.vao);
		}
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
    DestroyWindow();

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

	pipeline_batch_map_[pipeline->GetInstanceID()] = { pipeline, program_id, {} };
	return pipeline;
}

std::shared_ptr<Material> OpenGLRenderer::CreateMaterial(MaterialInfo info)
{
	const std::shared_ptr<Material> material = material_manager_.CreateMaterial(info);
	const PipelineID pipelineId = info.rendering_pipeline->GetInstanceID();
	std::unordered_map<PipelineID, PipelineBatch>::iterator iter = pipeline_batch_map_.find(pipelineId);
	if (iter != pipeline_batch_map_.end()) {
		PipelineBatch const* pipeline_batch = &iter->second;
		std::pair<std::unordered_map<MaterialID, MaterialBatch>::iterator, bool> material_batch_insertion = pipeline_batch->material_batch_map.insert(std::make_pair(material->GetInstanceID(), { material, 0, {} }));
		MaterialBatch& material_batch = material_batch_insertion.first->second;
		glGenVertexArrays(1, &material_batch.vao);
	}
	else {
		// This pipeline should have been registered in this class already when it was created.
	}
}

std::shared_ptr<Mesh> OpenGLRenderer::CreateMesh(MeshInfo info) 
{
	const std::shared_ptr<Mesh> mesh = mesh_manager_.CreateMesh(info);
	return mesh;
}

