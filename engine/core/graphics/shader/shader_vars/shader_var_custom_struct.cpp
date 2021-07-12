
#include "shader_var_custom_struct.h"

void* ShaderVarCustomStruct::GetData()
{
	return [value_.f.GetData(), value_.i.GetData()];
}
