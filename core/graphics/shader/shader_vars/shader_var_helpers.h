#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "something_shader/vars.h"
#include "shader_data_type.h"

namespace shader {

	namespace {
		// Serialization to compact bytes

		int SerializeToCompactBytes(float& f, char* bytes) {
			memcpy(bytes, &f, sizeof(f));
			return sizeof(f);
		}

		int SerializeToCompactBytes(glm::vec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::vec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::vec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			data_ptr += SerializeToCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(int& i, char* bytes) {
			memcpy(bytes, &i, sizeof(i));
			return sizeof(i);
		}

		int SerializeToCompactBytes(glm::ivec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::ivec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::ivec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			data_ptr += SerializeToCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(unsigned int& i, char* bytes) {
			memcpy(bytes, &i, sizeof(i));
			return sizeof(i);
		}

		int SerializeToCompactBytes(glm::uvec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::uvec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int SerializeToCompactBytes(glm::uvec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(v.x, data_ptr);
			data_ptr += SerializeToCompactBytes(v.y, data_ptr);
			data_ptr += SerializeToCompactBytes(v.z, data_ptr);
			data_ptr += SerializeToCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		// I am assuming below that the values of the glm matrices are tightly packed
		// in an array. C++ arrays have no padding between elements.

		int SerializeToCompactBytes(glm::mat2& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat3& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat4& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat2x3& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat3x2& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat2x4& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat4x2& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat3x4& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(glm::mat4x3& m, char* bytes) {
			memcpy(bytes, glm::value_ptr(m), sizeof(m));
			return sizeof(m);
		}

		int SerializeToCompactBytes(something_shader::CustomStruct& cs, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += SerializeToCompactBytes(cs.f, data_ptr);
			data_ptr += SerializeToCompactBytes(cs.v, data_ptr);
			return (data_ptr - bytes);
		}

		// Deserialization from compact bytes

		int DeserializeFromCompactBytes(float& f, char* bytes) {
			memcpy(&f, bytes, sizeof(f));
			return sizeof(f);
		}

		int DeserializeFromCompactBytes(glm::vec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::vec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::vec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(int& i, char* bytes) {
			memcpy(&i, bytes, sizeof(i));
			return sizeof(i);
		}

		int DeserializeFromCompactBytes(glm::ivec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::ivec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::ivec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(unsigned int& i, char* bytes) {
			memcpy(&i, bytes, sizeof(i));
			return sizeof(i);
		}

		int DeserializeFromCompactBytes(glm::uvec2& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::uvec3& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			return (data_ptr - bytes);
		}

		int DeserializeFromCompactBytes(glm::uvec4& v, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(v.x, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.y, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.z, data_ptr);
			data_ptr += DeserializeFromCompactBytes(v.w, data_ptr);
			return (data_ptr - bytes);
		}

		// I am assuming below that the values of the glm matrices are tightly packed
		// in an array. C++ arrays have no padding between elements.

		int DeserializeFromCompactBytes(glm::mat2& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat3& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat4& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat2x3& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat3x2& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat2x4& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat4x2& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat3x4& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(glm::mat4x3& m, char* bytes) {
			memcpy(glm::value_ptr(m), bytes, sizeof(m));
			return sizeof(m);
		}

		int DeserializeFromCompactBytes(something_shader::CustomStruct& cs, char* bytes) {
			char* data_ptr = bytes;
			data_ptr += DeserializeFromCompactBytes(cs.f, data_ptr);
			data_ptr += DeserializeFromCompactBytes(cs.v, data_ptr);
			return (data_ptr - bytes);
		}
	}


	/* Used for converting Uniform values to/from data */

	template<typename T>
	std::vector<char> SerializeValueToCompactBytes(T& value) {
		char bytes_buffer[sizeof(T)];
		int compact_size = SerializeToCompactBytes(value, bytes_buffer);
		const std::vector<char> bytes(bytes_buffer, bytes_buffer + compact_size);
		return bytes;
	}

	template<typename T>
	void DeserializeValueFromCompactBytes(T& value, std::vector<char>& bytes) {
		assert(bytes.size() <= sizeof(T));
		DeserializeFromCompactBytes(value, bytes.data());
	}

	/* Used for converting Vertex Attributes buffers to/from data. Note that we can do the following 
	   without worrying about alignment issues because glsl shader inputs are fundamental types. */

	template<typename T>
	std::vector<char> DataFromBuffer(std::vector<T>& buffer) {
		char* buffer_data_ptr = reinterpret_cast<char*>(buffer.data());
		const std::vector<char> buffer_data(buffer_data_ptr, buffer_data_ptr + (buffer.size() * sizeof(T)));
		return buffer_data;
	}

	template<typename T>
	std::vector<T> BufferFromData(std::vector<char>& data) {
		T* buffer_data_ptr = reinterpret_cast<T*>(data.data());
		const std::vector<T> buffer(buffer_data_ptr, buffer_data_ptr + (data.size() / sizeof(T)));
		return buffer;
	}

	ShaderDataType TypeID(float& f);
	ShaderDataType TypeID(glm::vec2& v);
	ShaderDataType TypeID(glm::vec3& v);
	ShaderDataType TypeID(glm::vec4& v);
	ShaderDataType TypeID(int& i);
	ShaderDataType TypeID(glm::ivec2& iv);
	ShaderDataType TypeID(glm::ivec3& iv);
	ShaderDataType TypeID(glm::ivec4& iv);
	ShaderDataType TypeID(unsigned int& ui);
	ShaderDataType TypeID(glm::uvec2& uv);
	ShaderDataType TypeID(glm::uvec3& uv);
	ShaderDataType TypeID(glm::uvec4& uv);
	ShaderDataType TypeID(glm::mat2& m);
	ShaderDataType TypeID(glm::mat3& m);
	ShaderDataType TypeID(glm::mat4& m);
	ShaderDataType TypeID(glm::mat2x3& m);
	ShaderDataType TypeID(glm::mat3x2& m);
	ShaderDataType TypeID(glm::mat2x4& m);
	ShaderDataType TypeID(glm::mat4x2& m);
	ShaderDataType TypeID(glm::mat3x4& m);
	ShaderDataType TypeID(glm::mat4x3& m);

	// OpenGL uniform helpers
	namespace opengl {
		void SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr);
	}
}