
#include "opengl_renderer.h"

std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info) 
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

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

		for (auto& it : material_batch_map)
		{
			MaterialBatch material_batch = it.second;
			glBindVertexArray(material_batch.vao);
			GLint mvp_location = glGetUniformLocation(material_batch.material.ProgramID(), "mvp");
			for (auto& it : material_batch.mesh_batch_map) {
				MeshBatch mesh_batch = it.second;
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

