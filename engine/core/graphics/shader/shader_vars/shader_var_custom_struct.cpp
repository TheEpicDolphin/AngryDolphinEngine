
#include "shader_var_custom_struct.h"

void* ShaderVarCustomStruct::GetData()
{
	return data_;
}

void ShaderVarCustomStruct::SetValue(CustomStruct value)
{
	f1_var.SetValue(value.f1);
	f2_var.SetValue(value.f2);

	memcpy(data_[0], f1_var_.GetData(), 4);
	memcpy(data_[4], f2_var_.GetData(), 4);
}

CustomStruct ShaderVarCustomStruct::GetValue()
{
	return {f1_var.GetValue(), f2_var.GetValue()};
}
