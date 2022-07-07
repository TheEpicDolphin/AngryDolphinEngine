
#include "opengl_renderer.h"

#include <string>
#include <map>

#include <glm/gtc/type_ptr.hpp>

OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer() {
	for (std::unordered_map<PipelineHandle, PipelineState>::iterator it = pipeline_state_map_.begin(); it != pipeline_state_map_.end(); it++) {
		it->second.pipeline->RemoveLifecycleEventsListener(this);
	}

	for (std::unordered_map<MeshHandle, MeshState>::iterator it = mesh_state_map_.begin(); it != mesh_state_map_.end(); it++) {
		it->second.mesh->RemoveLifecycleEventsListener(this);
	}

	for (std::unordered_map<MaterialHandle, MaterialState>::iterator it = material_state_map_.begin(); it != material_state_map_.end(); it++) {
		it->second.material->RemoveLifecycleEventsListener(this);
	}
}

void OpenGLRenderer::PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) {
	// This will avoid having any lags during rendering.
	if (pipeline_state_map_.find(pipeline.get()) != pipeline_state_map_.end()) {
		// This pipeline was already preloaded.
		return;
	}
	pipeline_state_map_[pipeline.get()] = CreatePipelineState(pipeline);
	pipeline->AddLifecycleEventsListener(this);
}

void OpenGLRenderer::RenderFrame(const CameraParams& camera_params, const std::vector<RenderableObject>& renderable_objects) {
	// TODO: using camera viewport rect value to set this.
	glViewport(
		(GLint)camera_params.viewport_rect.origin.x,
		(GLint)camera_params.viewport_rect.origin.y,
		(GLsizei)camera_params.viewport_rect.size.x, 
		(GLsizei)camera_params.viewport_rect.size.y
	);
	glClear(GL_COLOR_BUFFER_BIT);

	std::map<RenderableObjectBatchKey, RenderableObjectBatch> sorted_renderable_batches;
	for (RenderableObject renderable_object : renderable_objects) {
		const std::shared_ptr<RenderingPipeline>& pipeline = renderable_object.mesh->GetPipeline();

		if (pipeline_state_map_.find(pipeline.get()) == pipeline_state_map_.end()) {
			pipeline_state_map_[pipeline.get()] = CreatePipelineState(pipeline);
			pipeline->AddLifecycleEventsListener(this);
		}

		if (mesh_state_map_.find(renderable_object.mesh) == mesh_state_map_.end()) {
			mesh_state_map_[renderable_object.mesh] = CreateMeshState(renderable_object.mesh);
			renderable_object.mesh->AddLifecycleEventsListener(this);
		}

		if (material_state_map_.find(renderable_object.material) == material_state_map_.end()) {
			material_state_map_[renderable_object.material] = CreateMaterialState(renderable_object.material);
			renderable_object.material->AddLifecycleEventsListener(this);
		}

		const RenderableObjectBatchKey batch_key = {
			pipeline.get(),
			renderable_object.mesh,
			renderable_object.material
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

	const glm::mat4 vp = camera_params.view_projection_matrix;
	GLint mvp_location = 0;
	RenderableObjectBatchKey previous_batch_key = { 0, 0, 0 };
	for (std::map<RenderableObjectBatchKey, RenderableObjectBatch>::iterator it = sorted_renderable_batches.begin();
		it != sorted_renderable_batches.end();
		it++) {
		RenderableObjectBatchKey current_batch_key = it->first;
		RenderableObjectBatch batch = it->second;

		//GLint bones_location;
		if (current_batch_key.pipeline_handle != previous_batch_key.pipeline_handle) {
			// Switch rendering pipeline configuration
			const PipelineState pipeline_state = pipeline_state_map_[current_batch_key.pipeline_handle];
			glUseProgram(pipeline_state.program_id);

			mvp_location = pipeline_state.pipeline->MVPUniform().location;
			//bones_location = pipeline_state.pipeline->BonesUniform().location;
		}
		if (current_batch_key.mesh_handle != previous_batch_key.mesh_handle) {
			// Switch mesh configuration
			const MeshState mesh_state = mesh_state_map_[current_batch_key.mesh_handle];
			glBindVertexArray(mesh_state.vao);
		}
		if (current_batch_key.material_handle != previous_batch_key.material_handle) {
			// Switch material configuration
			// Iterate material uniforms and set corresponding uniforms in shaders
			for (std::size_t i = 0; i < batch.material->UniformValues().size(); i++) {
				const UniformInfo uniform_info = batch.mesh->GetPipeline()->MaterialUniforms()[i];
				const UniformValue uniform_value = batch.material->UniformValues()[i];
				shader::opengl::SetUniform(uniform_info.data_type, uniform_info.location, uniform_info.array_length, uniform_value.data.data());
			}
		}

		// Iterate over instances of mesh and draw each after transforming (and optionally setting bones).
		for (RenderableObjectInstance& renderable_object_instance : batch.instances) {
			const glm::mat4 mvp = renderable_object_instance.model_transform * vp;
			// Set MVP matrix in shader.
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
			/*
			if (bones_location) {
				// Set bones array in shader.
				glUniformMatrix4fv(
					bones_location,
					renderable_object_instance.bone_transforms.size(),
					GL_FALSE,
					reinterpret_cast<const GLfloat*>(renderable_object_instance.bone_transforms.data())
				);
			}
			*/

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

void OpenGLRenderer::Cleanup() {
	// TODO: Delete VAOs and VBOs
}

GLenum GLShaderTypeForStageType(shader::ShaderStageType type) 
{
	switch (type)
	{
	case shader::ShaderStageType::Vertex:
		return GL_VERTEX_SHADER;
	case shader::ShaderStageType::Geometry:
		return GL_GEOMETRY_SHADER;
	case shader::ShaderStageType::Fragment:
		return GL_FRAGMENT_SHADER;
	case shader::ShaderStageType::Compute:
		return GL_COMPUTE_SHADER;
	}
	return 0;
}

std::string NameForStageType(shader::ShaderStageType type)
{
	switch (type)
	{
	case shader::ShaderStageType::Vertex:
		return "Vertex Shader Stage";
	case shader::ShaderStageType::Geometry:
		return "Geometry Shader Stage";
	case shader::ShaderStageType::Fragment:
		return "Fragment Shader Stage";
	case shader::ShaderStageType::Compute:
		return "Compute Shader Stage";
	}
	return "";
}

OpenGLRenderer::PipelineState OpenGLRenderer::CreatePipelineState(const std::shared_ptr<RenderingPipeline>& pipeline)
{
	const std::vector<shader::Shader> shader_stages = pipeline->ShaderStages();
	GLuint program_id = glCreateProgram();
	std::vector<GLuint> shader_ids;
	shader_ids.reserve(shader_stages.size());
	GLint result = GL_FALSE;
	int info_log_length;
	for (shader::Shader shader : shader_stages) {
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

	return { pipeline.get(), program_id };
}



static inline void WriteVertexBufferData(GLuint& bo, std::vector<char> buffer_data) {
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER, buffer_data.size(), buffer_data.data(), GL_STATIC_DRAW);
}

OpenGLRenderer::MeshState OpenGLRenderer::CreateMeshState(Mesh* mesh) {
	MeshDataUsageType data_usage_type = mesh->IsStatic() ? MeshDataUsageType::Static : MeshDataUsageType::Dynamic;

	MeshState mesh_state = { mesh, data_usage_type, 0, 0, 0 };
	switch (data_usage_type) {
	case MeshDataUsageType::Static:
		// Fall through to Dynamic for now.
		// TODO: Implement this.
	case MeshDataUsageType::Dynamic:
		const std::shared_ptr<RenderingPipeline>& pipeline = mesh->GetPipeline();
		glGenVertexArrays(1, &mesh_state.vao);
		glBindVertexArray(mesh_state.vao);
		
		glGenBuffers(1, &mesh_state.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_state.ibo);
		const std::vector<std::size_t>& tri_indices = mesh->GetTriangleIndices();
		const char* element_array_buffer_data = reinterpret_cast<const char*>(tri_indices.data());
		glBufferData(GL_ARRAY_BUFFER, tri_indices.size() * sizeof(std::size_t), element_array_buffer_data, GL_STATIC_DRAW);

		const std::size_t num_va_buffers = mesh->GetVertexAttributeBuffers().size();
		mesh_state.bos = new GLuint[num_va_buffers];
		glGenBuffers(num_va_buffers, mesh_state.bos);

		for (std::size_t i = 0; i < num_va_buffers; i++) {
			const VertexAttributeInfo& vertex_attribute = pipeline->VertexAttributes()[i];
			const VertexAttributeBuffer& va_buffer = mesh->GetVertexAttributeBuffers()[i];
			
			WriteVertexBufferData(*(mesh_state.bos + i), va_buffer.data);

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
	return mesh_state;
}

OpenGLRenderer::MaterialState OpenGLRenderer::CreateMaterialState(Material* mat) {
	return { mat };
}

// PipelineLifecycleEventsListener

void OpenGLRenderer::PipelineDidDestroy(RenderingPipeline* pipeline) {
	pipeline_state_map_.erase(pipeline);
}

// MeshLifecycleEventsListener

void OpenGLRenderer::MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) {
	std::unordered_map<MeshHandle, MeshState>::iterator iter = mesh_state_map_.find(mesh);
	if (iter != mesh_state_map_.end()) {
		MeshState mesh_state = iter->second;

		const std::shared_ptr<RenderingPipeline>& pipeline = mesh->GetPipeline();
		glBindVertexArray(mesh_state.vao);
		WriteVertexBufferData(*(mesh_state.bos + attribute_index), mesh->GetVertexAttributeBuffers()[attribute_index].data);
		glBindVertexArray(0);
	}
	else {
		// This shouldn't happen
	}
}

void OpenGLRenderer::MeshDidDestroy(Mesh* mesh) {
	mesh_state_map_.erase(mesh);
}

// MaterialLifecycleEventsListener

void OpenGLRenderer::MaterialUniformDidChange(Material* material, std::size_t uniform_index) {
	// Do nothing for now.
}

/*
void OpenGLRenderer::MaterialTextureDidChange(Material* material, Texture texture) {
	// TODO
	std::unordered_map<MeshID, MaterialState>::iterator iter = material_state_map_.find(material->GetInstanceID());
	if (iter != material_state_map_.end()) {
		MaterialState mesh_state = iter->second;

	}
	else {
		// This shouldn't happen
	}
}
*/

void OpenGLRenderer::MaterialDidDestroy(Material* material) {
	material_state_map_.erase(material);
}
