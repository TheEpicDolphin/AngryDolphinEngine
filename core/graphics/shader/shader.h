#pragma once

#include <vector>

namespace shader {
	enum class ShaderStageType {
		Vertex = 0,
		Geometry,
		Fragment,
		Compute
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