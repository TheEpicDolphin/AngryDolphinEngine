#pragma once

#include <string>

#include "shader_var.h"

class Uniform
{

public:
	Uniform(std::string name, ShaderVarBase shader_var);

	const std::string& GetName();

	const ShaderVarBase& GetShaderVar();

private:
	std::string name_;
	ShaderVarBase shader_var_;
};