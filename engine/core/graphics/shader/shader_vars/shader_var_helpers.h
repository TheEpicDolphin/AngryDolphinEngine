#pragma once

#include "something_shader/vars.h"
#include "shader_data_type.h"

#include <vector>

namespace shader{

	static const std::size_t index_not_found = -1;

	/* Used for converting Uniform values to/from data */
	const std::vector<char> ValueData(float& f);
	const std::vector<char> ValueData(something_shader::CustomStruct& cs);
	void MakeValue(float* f, std::vector<char> data);
	void MakeValue(something_shader::CustomStruct* cs, std::vector<char> data);

	/* Used for converting Vertex Attributes buffers to data */
	const std::vector<char> BufferData(std::vector<float>& float_buffer);
	const std::vector<char> BufferData(std::vector<glm::vec3>& vec3_buffer);

	ShaderDataType TypeID(float& f);
	ShaderDataType TypeID(something_shader::CustomStruct& cs);
}