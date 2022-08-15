#pragma once

#include "../serializable.h"

class STLStringBuilder {
public:
	void OnDeconstructToParameters(const std::string& string);
	void OnConstructFromParameters(std::string& string);
	SERIALIZABLE_MEMBERS(DYNAMIC_ARRAY(c_string_, length_))

private:
	char* c_string_;
	std::size_t length_;
};
