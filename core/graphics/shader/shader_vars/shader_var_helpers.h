#pragma once

#include <string>
#include <vector>

#include "something_shader/vars.h"
#include "shader_data_type.h"

#define VERTEX_ATTRIBUTE_POSITION_NAME "_position"
#define VERTEX_ATTRIBUTE_NORMAL_NAME "_normal"
#define VERTEX_ATTRIBUTE_TEX_COORDS_NAME "_texCoords"
#define VERTEX_ATTRIBUTE_BONE_WEIGHTS_NAME "_boneWeights"
#define VERTEX_ATTRIBUTE_BONE_INDICES_NAME "_boneIndices"

namespace shader{

	const std::size_t index_not_found = -1;

	/* Used for converting Uniform values to/from data */
	const std::vector<char> ValueData(float& f);
	const std::vector<char> ValueData(something_shader::CustomStruct& cs);
	const std::vector<char> ValueArrayData(float f[]);
	const std::vector<char> ValueArrayData(something_shader::CustomStruct cs[]);

	void MakeValue(float* f, std::vector<char> data);
	void MakeValue(something_shader::CustomStruct* cs, std::vector<char> data);

	/* Used for converting Vertex Attributes buffers to data */
	const std::vector<char> BufferData(std::vector<float>& float_buffer);
	const std::vector<char> BufferData(std::vector<glm::vec3>& vec3_buffer);

	ShaderDataType TypeID(float& f);
	ShaderDataType TypeID(something_shader::CustomStruct& cs);

	// OpenGL uniform helpers
	namespace opengl {
		void SetUniform(ShaderDataType type, int location, int array_length, const char* value_ptr);
	}
}