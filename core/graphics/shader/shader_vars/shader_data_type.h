#pragma once

namespace shader {
	enum class ShaderDataType
	{
		Unknown = 0,
		Float,
		Vector2f,
		Vector3f,
		Vector4f,
		Int,
		Vector2i,
		Vector3i,
		Vector4i,
		UInt,
		Vector2ui,
		Vector3ui,
		Vector4ui,
		Matrix2f,
		Matrix3f,
		Matrix4f,
		Matrix2x3f,
		Matrix3x2f,
		Matrix2x4f,
		Matrix4x2f,
		Matrix3x4f,
		Matrix4x3f,
		SomethingShaderCustomStruct,
	};
}
