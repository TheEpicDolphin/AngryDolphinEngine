#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>
#include <core/object/object.h>

#include "shader.h"

class Material : Object<Material>
{
	Material() 
	{
		// THIS IS TEMPORARY. Later, load shaders at startup only
		program_id_ = LoadShaders("", "");
	}

private:
	GLuint program_id_;
};