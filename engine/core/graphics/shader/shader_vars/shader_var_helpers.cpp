
#include "shader_var_helpers.h"

const std::vector<char> shader::ValueData(float& f)
{
	const std::vector<char> data(sizeof(f));
	std::vector<char>::insert(data.begin(), sizeof(f), f);
	return data;
}

const std::vector<char> shader::ValueData(something_shader::CustomStruct& cs)
{

	return;
}

ShaderDataType shader::TypeID(float& f) { return ShaderDataTypeFloat; }
ShaderDataType shader::TypeID(something_shader::CustomStruct& cs) { return ShaderDataTypeSomethingShaderCustomStruct; }

void shader::MakeValue(float* f, std::vector<char> data) 
{

}

void shader::MakeValue(something_shader::CustomStruct* cs, std::vector<char> data) 
{

}