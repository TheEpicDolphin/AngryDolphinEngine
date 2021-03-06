#pragma once

#include "../shader_var.h"

class ShaderVarFloat : public ShaderVar<float>
{
public:
	void* GetData() override;

	void SetValue(float value) override;

	float GetValue() override;

private:
	float value_;
};