
#include "shader_var_float.h"

void* ShaderVarFloat::GetData() 
{
	return &value_;
}

void ShaderVarFloat::SetValue(float value) 
{
	value_ = value;
}

float ShaderVarFloat::GetValue()
{
	return value_;
}
