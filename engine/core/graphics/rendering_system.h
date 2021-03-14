#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>
#include <GL/glew.h>

#include "mesh_renderer.h"

class RenderingSystem : public System<RenderingSystem>
{
private:
	struct MeshBatch {
		GLuint vbo;
		GLuint ibo;
		Mesh mesh;
		std::vector<glm::mat4> trans_matrices;
	};

	struct MaterialBatch {
		GLuint vao;
		Material material;
		std::unordered_map<InstanceID, MeshBatch> mesh_batch_map;
	};

	

public:
	RenderingSystem(ECS ecs) : System<RenderingSystem>(ecs) {

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
				MeshBatch mesh_batch = {
					0,
					0,
					*mesh_rend.mesh.get(),
					{ trans.matrix }
				};
				material_batch.mesh_batch_map[meshID] = mesh_batch;
				material_batch_map[materialID] = material_batch;

				glGenVertexArrays(1, &material_batch_map[materialID].vao);
				glBindVertexArray(material_batch.vao);

				glUseProgram(material_batch.material.ProgramID());

				glGenBuffers(1, &mesh_batch.vbo);
				//glGenBuffers(1, &mesh_batch.ibo);
				glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_batch.ibo);

				glBufferData(GL_ARRAY_BUFFER, 3, mesh_batch.mesh.GetVertices().data, GL_STATIC_DRAW);
				
				glEnableVertexAttribArray(material_batch.material.VertexAttribute());
				
				glVertexAttribPointer(
					material_batch.material.VertexAttribute(), // The shader's attribute for vertex positions.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				glDisableVertexAttribArray(material_batch.material.VertexAttribute());
			}
			else {
				MaterialBatch *material_batch = &material_batch_map[materialID];
				if (material_batch->mesh_batch_map.find(meshID) == material_batch->mesh_batch_map.end()) {
					MeshBatch mesh_batch = {
						0,
						*mesh_rend.mesh.get(),
						{ trans.matrix }
					};
					material_batch->mesh_batch_map[meshID] = mesh_batch;
					glBindVertexArray(material_batch->vao);
					glGenBuffers(1, &mesh_batch.vbo);
					glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
				}
				else {
					MeshBatch *mesh_batch = &material_batch->mesh_batch_map[meshID];
					mesh_batch->trans_matrices.push_back(trans.matrix);
				}
			}
		};
		ecs_.EnumerateComponentsWithBlock<MeshRenderer, Transform>(block);

		for (auto& it : material_batch_map)
		{
			MaterialBatch material_batch = it.second;
			glBindVertexArray(material_batch.vao);
			GLint mvp_location = glGetUniformLocation(material_batch.material.ProgramID(), "mvp");
			for (auto& it : material_batch.mesh_batch_map) {
				MeshBatch mesh_batch = it.second;
				for (glm::mat4& trans_matrix : mesh_batch.trans_matrices) {
					// Insert MVP matrix into shader
					glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(trans_matrix));
					// Draw
					glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
				}
				glDeleteBuffers(1, &mesh_batch.vbo);
			}
			glDeleteVertexArrays(1, &material_batch.vao);
		}

		// TODO: Do above inside window context
	}

	void SetupBufferObjectsForBatch(MaterialBatch& materialBatch) {
		MaterialBatch material_batch = { 0 };
		material_batch.material = *mesh_rend.material.get();
		MeshBatch mesh_batch = {
			0,
			0,
			*mesh_rend.mesh.get(),
			{ trans.matrix }
		};
		material_batch.mesh_batch_map[meshID] = mesh_batch;
		material_batch_map[materialID] = material_batch;

		glGenVertexArrays(1, &material_batch_map[materialID].vao);
		glBindVertexArray(material_batch.vao);

		glUseProgram(material_batch.material.ProgramID());

		glGenBuffers(1, &mesh_batch.vbo);
		//glGenBuffers(1, &mesh_batch.ibo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_batch.ibo);

		glBufferData(GL_ARRAY_BUFFER, 3, mesh_batch.mesh.GetVertices().data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(material_batch.material.VertexAttribute());

		glVertexAttribPointer(
			material_batch.material.VertexAttribute(), // The shader's attribute for vertex positions.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glDisableVertexAttribArray(material_batch.material.VertexAttribute());
	}
};