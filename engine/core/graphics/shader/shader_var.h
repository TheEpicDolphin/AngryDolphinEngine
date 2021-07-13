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

	virtual void SetValue(T value) = 0;

	virtual T GetValue() = 0;

private:
	static ShaderDataType type_;
};
