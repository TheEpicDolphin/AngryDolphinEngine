#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>
#include <GL/glew.h>

#include "camera.h"
#include "mesh_renderer.h"
#include "render_context.h"

class RenderingSystem : public System<RenderingSystem>
{
private:
	IRenderContext render_context_;

	struct MeshBatch {
		GLuint vbo;
		GLuint ibo;
		Mesh mesh;
		std::vector<glm::mat4> model_matrices;
	};

	struct MaterialBatch {
		GLuint vao;
		Material material;
		std::unordered_map<InstanceID, MeshBatch> mesh_batch_map;
	};

	MeshBatch CreateMeshBatch(GLuint& vao, Material& material, Mesh& mesh) {
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

public:
	RenderingSystem() = default;

	RenderingSystem(ECS *ecs) : System<RenderingSystem>(ecs) {
		
	}

	void Update() 
	{
		std::unordered_map<InstanceID, MaterialBatch> material_batch_map;

		std::function<void(EntityID, MeshRenderer&, Transform&)> block =
		[&](EntityID entity_id, MeshRenderer& mesh_rend, Transform& trans) {
			// group together materials and meshes for faster rendering later
			InstanceID materialID = mesh_rend.material->GetInstanceID();
			InstanceID meshID = mesh_rend.mesh->GetInstanceID();
			if (material_batch_map.find(materialID) == material_batch_map.end()) {
				MaterialBatch material_batch = {0};
				material_batch.material = *mesh_rend.material.get();

				glGenVertexArrays(1, &material_batch.vao);
				MeshBatch mesh_batch = CreateMeshBatch(material_batch.vao, *mesh_rend.material.get(), *mesh_rend.mesh.get());
				mesh_batch.model_matrices.push_back(trans.matrix);

				material_batch.mesh_batch_map[meshID] = mesh_batch;
				material_batch_map[materialID] = material_batch;
			}
			else {
				MaterialBatch *material_batch = &material_batch_map[materialID];
				if (material_batch->mesh_batch_map.find(meshID) == material_batch->mesh_batch_map.end()) {
					MeshBatch mesh_batch = CreateMeshBatch(material_batch->vao, *mesh_rend.material.get(), *mesh_rend.mesh.get());
					mesh_batch.model_matrices.push_back(trans.matrix);

					material_batch->mesh_batch_map[meshID] = mesh_batch;
				}
				else {
					MeshBatch *mesh_batch = &material_batch->mesh_batch_map[meshID];
					mesh_batch->model_matrices.push_back(trans.matrix);
				}
			}
		};
		ecs_->EnumerateComponentsWithBlock<MeshRenderer, Transform>(block);

		int window_width, window_height;
		render_context_.FrameBufferSize(&window_width, &window_height);
		glViewport(0, 0, window_width, window_height);
		glClear(GL_COLOR_BUFFER_BIT);
		std::function<void(EntityID, Camera&, Transform&)> rendering_block =
			[&](EntityID entity_id, Camera& camera, Transform& transform) {
			if (!camera.enabled) {
				// Camera is disabled, don't render
				return;
			}

			const glm::mat4 view_matrix = transform.matrix;
			glm::mat4 projection_matrix;
			if (camera.is_orthographic) {
				// Orthographic projection matrix
				projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);
			} else {
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
		};
		ecs_->EnumerateComponentsWithBlock<Camera, Transform>(rendering_block);
	}
};