#pragma once

#include <vector>

namespace shader {
	enum ShaderStageType {
		ShaderStageTypeVertex = 0,
		ShaderStageTypeGeometry,
		ShaderStageTypeFragment,
		ShaderStageTypeCompute
	};

	struct Shader
	{
		ShaderStageType type;
		std::vector<char> code;

		Shader(ShaderStageType type, std::vector<char> code) {
			this->type = type;
			this->code = code;
		}
	};
}