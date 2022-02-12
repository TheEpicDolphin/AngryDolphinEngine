#pragma once

#include <string>
#include <vector>

#include "something_shader/vars.h"
#include "shader_data_type.h"

namespace shader{

	const std::size_t index_not_found = -1;

	/* Used for converting Uniform values to/from data */

	template<typename T>
	std::vector<char> ValueData(T& value) {
		char* value_data_ptr = reinterpret_cast<char*>(&value);
		const std::vector<char> value_data(value_data_ptr, value_data_ptr + sizeof(T));
		return value_data;
	}

	template<typename T>
	std::vector<char> ValueArrayData(T& value[]) {
		char* value_data_ptr = reinterpret_cast<char*>(&value);
		const std::vector<char> value_data(value_data_ptr, value_data_ptr + sizeof(T));
		return value_data;
	}

	template<typename T>
	void MakeValue(T* value, std::vector<char>& data) {
		int value;
		assert(sizeof(T) == data.size());
		std::memcpy(&value, data.data(), sizeof(T));
	}

	/* Used for converting Vertex Attributes buffers to data */

	template<typename T>
	std::vector<char> BufferData(std::vector<T>& buffer) {
		char* buffer_data_ptr = reinterpret_cast<char*>(buffer.data());
		const std::vector<char> buffer_data(buffer_data_ptr, buffer_data_ptr + (buffer.size() * sizeof(T)));
		return buffer_data;
	}

	ShaderDataType TypeID(float& f);
	ShaderDataType TypeID(something_shader::CustomStruct& cs);

	// OpenGL uniform helpers
	namespace opengl {
		void SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr);
	}
}