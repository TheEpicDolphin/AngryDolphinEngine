#pragma once

class TypeInfo 
{
public:
	template<typename T>
	std::uint32_t GetTypeId()
	{
		static std::uint32_t type_id = ++next_id_;
		return type_id;
	}

private:
	std::uint32_t next_id_;
};