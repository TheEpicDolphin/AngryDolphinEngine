
#include "shader.h"

Shader::Shader(ShaderStageType type, std::vector<char> code);
{
	this.type = type;
	this.code = code;
}
