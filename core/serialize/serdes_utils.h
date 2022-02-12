#pragma once

#include <sstream>

namespace serialize {

	template<typename T>
	void DeserializeArithmeticFromString(T& value, char* string) {
		std::stringstream tmp_ss;
		tmp_ss << string;
		tmp_ss >> value;
	}
}