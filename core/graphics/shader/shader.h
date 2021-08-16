#pragma once

#include <vector>

enum ShaderStageType { 
	ShaderStageTypeVertex = 0, 
	ShaderStageTypeGeometry,
	ShaderStageTypeFragment,
	ShaderStageTypeCompute
};

struct Shader 
{
	Shader(ShaderStageType type, std::vector<char> code);
	ShaderStageType type;
	std::vector<char> code;
};