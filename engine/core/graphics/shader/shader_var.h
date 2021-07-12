#pragma once

#include "shader_data_types/shader_data_type.h"
#include "shader_data_type_assigner/shader_data_type_assigner.h"

class ShaderVarBase
{
	virtual void* GetData();
};

template<typename T>
class ShaderVar : public ShaderVarBase
{
	friend class ShaderDataTypeAssigner;

public:
	ShaderDataType GetType() 
	{
		return type_;
	}

	void SetValue(T value) 
	{
		value_ = value;
	}

	T GetValue() 
	{
		return value_;
	}

private:
	static ShaderDataType type_;

protected:
	T value_;
};
