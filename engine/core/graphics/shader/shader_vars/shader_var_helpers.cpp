
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

int shader::TypeID(float& f) { return 1; }
int shader::TypeID(something_shader::CustomStruct& cs) { return 2; }

void shader::MakeValue(float* f, std::vector<char> data) 
{

}

void shader::MakeValue(something_shader::CustomStruct* cs, std::vector<char> data) 
{

}