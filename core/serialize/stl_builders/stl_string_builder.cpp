#include "stl_string_builder.h"

void STLStringBuilder::OnDeconstructToParameters(const std::string& string) {
	const std::size_t str_length = string.length();
	char_array_.reset(new char[str_length], str_length);
	memcpy(char_array_.begin(), string.c_str(), str_length);
}

void STLStringBuilder::OnConstructFromParameters(std::string& string) {
	string = std::string(char_array_.begin(), char_array_.end());
}