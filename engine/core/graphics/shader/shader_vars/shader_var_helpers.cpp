
#include "shader_var_helpers.h"

#include <GL/glew.h>

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

const std::vector<char> shader::ValueArrayData(float f[]) 
{

}

const std::vector<char> shader::ValueArrayData(something_shader::CustomStruct cs[]) 
{

}

ShaderDataType shader::TypeID(float& f) { return ShaderDataTypeFloat; }
ShaderDataType shader::TypeID(something_shader::CustomStruct& cs) { return ShaderDataTypeSomethingShaderCustomStruct; }

void shader::MakeValue(float* f, std::vector<char> data) 
{

}

void shader::MakeValue(something_shader::CustomStruct* cs, std::vector<char> data) 
{

}

void shader::opengl::SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr)
{
	switch (type)
	{
	case ShaderDataTypeFloat:
		glUniform1fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeVector2f:
		glUniform2fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeVector3f:
		glUniform3fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeVector4f:
		glUniform4fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeInt:
		glUniform1iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataTypeVector2i:
		glUniform2iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataTypeVector3i:
		glUniform3iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataTypeVector4i:
		glUniform4iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataTypeUInt:
		glUniform1uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataTypeVector2ui:
		glUniform2uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataTypeVector3ui:
		glUniform3uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataTypeVector4ui:
		glUniform4uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataTypeMatrix2f:
		glUniformMatrix2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix3f:
		glUniformMatrix3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix4f:
		glUniformMatrix4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix2x3f:
		glUniformMatrix2x3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix3x2f:
		glUniformMatrix3x2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix2x4f:
		glUniformMatrix2x4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix4x2f:
		glUniformMatrix4x2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix3x4f:
		glUniformMatrix3x4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeMatrix4x3f:
		glUniformMatrix4x3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataTypeSomethingShaderCustomStruct:

		break;
	}
}