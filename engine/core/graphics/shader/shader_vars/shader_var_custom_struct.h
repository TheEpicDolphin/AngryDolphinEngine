#pragma once

#include <vector>
#include "../shader_var.h"
#include "shader_var_float.h"

#define CUSTOM_STRUCT_SIZE 8

struct CustomStruct 
{
	float f1;
	float f2;
};

class ShaderVarCustomStruct : public ShaderVar<CustomStruct>
{
public:
	void* GetData() override;

	void SetValue(CustomStruct value) override;

	CustomStruct GetValue() override;

private:
	ShaderVarFloat f1_var_;
	ShaderVarFloat f2_var_;
	unsigned char data_[CUSTOMSTRUCT_SIZE];
};