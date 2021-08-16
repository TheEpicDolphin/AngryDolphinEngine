#pragma once

enum ShaderDataType 
{ 
	ShaderDataTypeUnknown = 0,
	ShaderDataTypeFloat,
	ShaderDataTypeVector2f,
	ShaderDataTypeVector3f,
	ShaderDataTypeVector4f,
	ShaderDataTypeInt,
	ShaderDataTypeVector2i,
	ShaderDataTypeVector3i,
	ShaderDataTypeVector4i,
	ShaderDataTypeUInt,
	ShaderDataTypeVector2ui,
	ShaderDataTypeVector3ui,
	ShaderDataTypeVector4ui,
	ShaderDataTypeMatrix2f,
	ShaderDataTypeMatrix3f,
	ShaderDataTypeMatrix4f,
	ShaderDataTypeMatrix2x3f,
	ShaderDataTypeMatrix3x2f,
	ShaderDataTypeMatrix2x4f,
	ShaderDataTypeMatrix4x2f,
	ShaderDataTypeMatrix3x4f,
	ShaderDataTypeMatrix4x3f,
	ShaderDataTypeSomethingShaderCustomStruct,
};