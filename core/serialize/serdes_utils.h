#pragma once

#include <sstream>

namespace serialize {

	template<typename T>
	std::string SerializeArithmeticToString(const T& value) {
		std::stringstream tmp_ss;
		tmp_ss << value;
		return tmp_ss.str();
	}

	template<typename T>
	std::string SerializeArithmeticToString(T&& value) {
		std::stringstream tmp_ss;
		tmp_ss << value;
		return tmp_ss.str();
	}

	template<typename T>
	void DeserializeArithmeticFromString(T& value, char* string) {
		std::stringstream tmp_ss;
		tmp_ss << string;
		tmp_ss >> value;
	}

	template<typename T>
	T DeserializeArithmeticFromString(char* string) {
		T value;
		std::stringstream tmp_ss;
		tmp_ss << string;
		tmp_ss >> value;
		return value;
	}
}