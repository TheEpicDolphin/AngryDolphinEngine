#pragma once

class TypeIDMapper
{
public:
	template<typename T>
	int GetTypeId()
	{
		static int type_id = ++next_id_;
		return type_id;
	}

private:
	int next_id_;
};