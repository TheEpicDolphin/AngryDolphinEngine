#pragma once

#include <string>

//#include <core/serialize/serializable.h>

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
		std::string code;

		//SERIALIZE_MEMBERS(type, code)
	};
}