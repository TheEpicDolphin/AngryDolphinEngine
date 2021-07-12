#pragma once

#include <string>

#include "shader_var.h"

struct Uniform
{
	ShaderVarBase shader_var;
	std::string name;
};