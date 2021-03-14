#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>
#include <core/object/object.h>

#include "shader.h"

class Material : public Object<Material>
{
public:
	Material() 
	{
		// THIS IS TEMPORARY. Later, load shaders at startup only
		program_id_ = LoadShaders("", "");
	}

	~Material() 
	{

	}

	GLuint ProgramID() 
	{
		return program_id_;
	}

	GLuint VertexAttribute() {
		return vertex_attribute_;
	}

private:
	GLuint program_id_;
	GLuint vertex_attribute_ = 0;
};