
#include "uniform.h"

Uniform::Uniform(std::string name, ShaderVarBase shader_var) 
{
	name_ = name;
	shader_var_ = shader_var;
}

const std::string& Uniform::GetName() 
{
	return name_;
}

const ShaderVarBase& Uniform::GetShaderVar() 
{
	return shader_var_;
}