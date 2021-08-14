#pragma once

enum ShaderDataType 
{ 
	ShaderDataTypeUnknown = 0,
	ShaderDataTypeFloat,
	ShaderDataTypeVector2f,
	ShaderDataTypeVector3f,
	ShaderDataTypeVector4f,
	ShaderDataTypeMatrix2f,
	ShaderDataTypeMatrix3f,
	ShaderDataTypeMatrix4f,
	ShaderDataTypeSomethingShaderCustomStruct,
};