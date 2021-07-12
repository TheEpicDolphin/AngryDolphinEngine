#pragma once

#include "../shader_data_types/shader_data_type.h"
#include "../shader_var.h"

class ShaderDataTypeAssigner 
{
	ShaderVar<float>::type_ = ShaderDataTypeFloat;
};