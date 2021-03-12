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
		Mesh mesh;
		std::vector<glm::mat4> trans_matrices;
	};

	struct MaterialBatch {
		GLuint vao;
		Material material;
		std::unordered_map<InstanceID, MeshBatch> mesh_batch_map;
	};

	std::unordered_map<InstanceID, MaterialBatch> material_batch_map_;

public:
	RenderingSystem(ECS ecs) : System<RenderingSystem>(ecs) {

	}

	void Update() 
	{
		std::function<void(EntityID, MeshRenderer&, Transform&)> block =
		[this](EntityID entity_id, MeshRenderer& mesh_rend, Transform& trans) {
			// group together materials and meshes for faster rendering later
			InstanceID materialID = mesh_rend.material->GetInstanceID();
			InstanceID meshID = mesh_rend.mesh->GetInstanceID();
			if (material_batch_map_.find(materialID) == material_batch_map_.end()) {
				MaterialBatch material_batch = {0};
				material_batch.material = *mesh_rend.material.get();
				MeshBatch mesh_batch = {
					0,
					*mesh_rend.mesh.get(),
					{ trans.matrix }
				};
				material_batch.mesh_batch_map[meshID] = mesh_batch;
				material_batch_map_[materialID] = material_batch;
				glGenVertexArrays(1, &material_batch_map_[materialID].vao);
			}
			else {
				MaterialBatch *material_batch = &material_batch_map_[materialID];
				if (material_batch->mesh_batch_map.find(meshID) == material_batch->mesh_batch_map.end()) {
					MeshBatch mesh_batch = {
						0,
						*mesh_rend.mesh.get(),
						{ trans.matrix }
					};
					material_batch->mesh_batch_map[meshID] = mesh_batch;
					glBindVertexArray(material_batch->vao);
					glBindBuffer(GL_ARRAY_BUFFER, mesh_batch.vbo);
				}
				else {
					MeshBatch *mesh_batch = &material_batch->mesh_batch_map[meshID];
					mesh_batch->trans_matrices.push_back(trans.matrix);
				}
			}
		};
		ecs_.EnumerateComponentsWithBlock<MeshRenderer, Transform>(block);


		GLuint vertexArrayID;
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// The following commands will talk about our 'vertexbuffer' buffer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// Give our vertices to OpenGL.
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		// Draw
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);


		// TODO: Do above inside window context
	}

	void CreateVAO() {

	}
};