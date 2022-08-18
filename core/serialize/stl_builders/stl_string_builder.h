#pragma once

#include "../serializable.h"

class STLStringBuilder {
public:
	void OnDeconstructToParameters(const std::string& string);
	void OnConstructFromParameters(std::string& string);
	SERIALIZABLE_MEMBERS(VAR(char_array_))

private:
	dynamic_memory_array<char> char_array_;
};
