
#include "shader_var_helpers.h"

#include <GL/glew.h>

using namespace shader;

ShaderDataType shader::TypeID(float& f) { return ShaderDataTypeFloat; }
ShaderDataType shader::TypeID(glm::vec2& v) { return ShaderDataTypeVector2f; }
ShaderDataType shader::TypeID(glm::vec3& v) { return ShaderDataTypeVector3f; }
ShaderDataType shader::TypeID(glm::vec4& v) { return ShaderDataTypeVector4f; }
ShaderDataType shader::TypeID(int& i) { return ShaderDataTypeInt; }
ShaderDataType shader::TypeID(glm::ivec2& iv) { return ShaderDataTypeVector2i; }
ShaderDataType shader::TypeID(glm::ivec3& iv) { return ShaderDataTypeVector3i; }
ShaderDataType shader::TypeID(glm::ivec4& iv) { return ShaderDataTypeVector4i; }
ShaderDataType shader::TypeID(unsigned int& ui) { return ShaderDataTypeUInt; }
ShaderDataType shader::TypeID(glm::uvec2& uv) { return ShaderDataTypeVector2ui; }
ShaderDataType shader::TypeID(glm::uvec3& uv) { return ShaderDataTypeVector3ui; }
ShaderDataType shader::TypeID(glm::uvec4& uv) { return ShaderDataTypeVector4ui; }
ShaderDataType shader::TypeID(glm::mat2& m) { return ShaderDataTypeMatrix2f; }
ShaderDataType shader::TypeID(glm::mat3& m) { return ShaderDataTypeMatrix3f; }
ShaderDataType shader::TypeID(glm::mat4& m) { return ShaderDataTypeMatrix4f; }
ShaderDataType shader::TypeID(glm::mat2x3& m) { return ShaderDataTypeMatrix2x3f; }
ShaderDataType shader::TypeID(glm::mat3x2& m) { return ShaderDataTypeMatrix3x2f; }
ShaderDataType shader::TypeID(glm::mat2x4& m) { return ShaderDataTypeMatrix2x4f; }
ShaderDataType shader::TypeID(glm::mat4x2& m) { return ShaderDataTypeMatrix4x2f; }
ShaderDataType shader::TypeID(glm::mat3x4& m) { return ShaderDataTypeMatrix3x4f; }
ShaderDataType shader::TypeID(glm::mat4x3& m) { return ShaderDataTypeMatrix4x3f; }

bool IsFundamentalGLSLType(ShaderDataType shader_data_type) {
	return !(shader_data_type > ShaderDataTypeMatrix4x3f);
}

void opengl::SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr)
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