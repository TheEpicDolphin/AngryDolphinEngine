#pragma once

#include "../shader_var.h"

struct CustomStruct 
{
	ShaderVarFloat f;
	ShaderVarInt i;
};

class ShaderVarCustomStruct : public ShaderVar<CustomStruct>
{
public:
	void* GetData() override;
};