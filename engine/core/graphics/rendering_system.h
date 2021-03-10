#pragma once

#include <iostream>
#include <map>
#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>
#include <GL/glew.h>

#include "mesh_renderer.h"

class RenderingSystem : public System<RenderingSystem>
{
public:
	RenderingSystem(ECS ecs) : System<RenderingSystem>(ecs) {

	}

	void Update() 
	{
		std::function<void(EntityID, MeshRenderer&, Transform&)> block =
		[](EntityID entity_id, MeshRenderer& mesh_rend, Transform& trans) {
			// TODO: render each entity

			if (!vao_map_.Contains()) {

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

private:
	std::map<uint64_t, GLuint> vao_map_;
};