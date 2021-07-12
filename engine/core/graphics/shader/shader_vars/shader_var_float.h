#pragma once

#include "../shader_var.h"

class ShaderVarFloat : public ShaderVar<float>
{
public:
	void* GetData() override;
};