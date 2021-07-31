#pragma once

#include "something_shader/vars.h"

#include <vector>

namespace shader{
	const std::vector<char> ValueData(float& f);
	const std::vector<char> ValueData(something_shader::CustomStruct& cs);

	int TypeID(float& f);
	int TypeID(something_shader::CustomStruct& cs);

	void MakeValue(float* f, std::vector<char> data);
	void MakeValue(something_shader::CustomStruct* cs, std::vector<char> data);
}