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

private:
	GLuint program_id_;
};