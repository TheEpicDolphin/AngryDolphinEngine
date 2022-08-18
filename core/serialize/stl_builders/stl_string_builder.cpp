#include "stl_string_builder.h"

void STLStringBuilder::OnDeconstructToParameters(const std::string& string) {
	const std::size_t str_length = string.length();
	char_array_.array_ptr = new char[str_length];
	char_array_.length = str_length;
	memcpy(char_array_.array_ptr, string.c_str(), str_length);
}

void STLStringBuilder::OnConstructFromParameters(std::string& string) {
	string = std::string(char_array_.begin(), char_array_.end());
}