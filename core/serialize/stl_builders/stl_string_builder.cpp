#include "stl_string_builder.h"

void STLStringBuilder::OnDeconstructToParameters(const std::string& string) {
	length_ = string.length();
	c_string_ = new char[length_];
	memcpy(c_string_, string.c_str(), length_);
}

void STLStringBuilder::OnConstructFromParameters(std::string& string) {
	string = std::string(c_string_, c_string_ + length_);
}