
#include "shader_var_helpers.h"

#include <GL/glew.h>

using namespace shader;

ShaderDataType shader::TypeID(float& f) { return ShaderDataType::Float; }
ShaderDataType shader::TypeID(glm::vec2& v) { return ShaderDataType::Vector2f; }
ShaderDataType shader::TypeID(glm::vec3& v) { return ShaderDataType::Vector3f; }
ShaderDataType shader::TypeID(glm::vec4& v) { return ShaderDataType::Vector4f; }
ShaderDataType shader::TypeID(int& i) { return ShaderDataType::Int; }
ShaderDataType shader::TypeID(glm::ivec2& iv) { return ShaderDataType::Vector2i; }
ShaderDataType shader::TypeID(glm::ivec3& iv) { return ShaderDataType::Vector3i; }
ShaderDataType shader::TypeID(glm::ivec4& iv) { return ShaderDataType::Vector4i; }
ShaderDataType shader::TypeID(unsigned int& ui) { return ShaderDataType::UInt; }
ShaderDataType shader::TypeID(glm::uvec2& uv) { return ShaderDataType::Vector2ui; }
ShaderDataType shader::TypeID(glm::uvec3& uv) { return ShaderDataType::Vector3ui; }
ShaderDataType shader::TypeID(glm::uvec4& uv) { return ShaderDataType::Vector4ui; }
ShaderDataType shader::TypeID(glm::mat2& m) { return ShaderDataType::Matrix2f; }
ShaderDataType shader::TypeID(glm::mat3& m) { return ShaderDataType::Matrix3f; }
ShaderDataType shader::TypeID(glm::mat4& m) { return ShaderDataType::Matrix4f; }
ShaderDataType shader::TypeID(glm::mat2x3& m) { return ShaderDataType::Matrix2x3f; }
ShaderDataType shader::TypeID(glm::mat3x2& m) { return ShaderDataType::Matrix3x2f; }
ShaderDataType shader::TypeID(glm::mat2x4& m) { return ShaderDataType::Matrix2x4f; }
ShaderDataType shader::TypeID(glm::mat4x2& m) { return ShaderDataType::Matrix4x2f; }
ShaderDataType shader::TypeID(glm::mat3x4& m) { return ShaderDataType::Matrix3x4f; }
ShaderDataType shader::TypeID(glm::mat4x3& m) { return ShaderDataType::Matrix4x3f; }

bool IsFundamentalGLSLType(ShaderDataType shader_data_type) {
	return !(shader_data_type > ShaderDataType::Matrix4x3f);
}

void opengl::SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr)
{
	switch (type)
	{
	case ShaderDataType::Float:
		glUniform1fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Vector2f:
		glUniform2fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Vector3f:
		glUniform3fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Vector4f:
		glUniform4fv(location, array_length, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Int:
		glUniform1iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataType::Vector2i:
		glUniform2iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataType::Vector3i:
		glUniform3iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataType::Vector4i:
		glUniform4iv(location, array_length, reinterpret_cast<const GLint*>(value_ptr));
		break;
	case ShaderDataType::UInt:
		glUniform1uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataType::Vector2ui:
		glUniform2uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataType::Vector3ui:
		glUniform3uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataType::Vector4ui:
		glUniform4uiv(location, array_length, reinterpret_cast<const GLuint*>(value_ptr));
		break;
	case ShaderDataType::Matrix2f:
		glUniformMatrix2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix3f:
		glUniformMatrix3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix4f:
		glUniformMatrix4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix2x3f:
		glUniformMatrix2x3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix3x2f:
		glUniformMatrix3x2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix2x4f:
		glUniformMatrix2x4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix4x2f:
		glUniformMatrix4x2fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix3x4f:
		glUniformMatrix3x4fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::Matrix4x3f:
		glUniformMatrix4x3fv(location, array_length, GL_FALSE, reinterpret_cast<const GLfloat*>(value_ptr));
		break;
	case ShaderDataType::SomethingShaderCustomStruct:
		
		break;
	}
}