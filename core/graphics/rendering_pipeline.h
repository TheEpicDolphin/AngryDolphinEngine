#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "shader/shader.h"

typedef std::uint32_t PipelineID;

enum VertexAttributeUsageCategory
{
	VertexAttributeUsageCategoryPosition = 0,
	VertexAttributeUsageCategoryNormal,
	VertexAttributeUsageCategoryTextureCoordinates,
	VertexAttributeUsageCategoryBoneWeights,
	VertexAttributeUsageCategoryBoneIndices,
	VertexAttributeUsageCategoryCustom,
};

struct RenderingPipelineInfo
{
	std::vector<Shader> shader_stages;
};

struct UniformInfo {
	// Name of this uniform.
	std::string name;
	// Data type. In the case of an array, this is the type of each element.
	ShaderDataType type;
	// Location in the shader.
	int location;
	// Array length. It is 1 for non-arrays.
	int array_length;
};

struct VertexAttributeInfo {
	// Name of this vertex attribute.
	std::string name;
	// Data type.
	ShaderDataType type;
	// Location in the shader.
	int location;
	// Number of components in the data type. For example, vec3 has a dimension of 3.
	int dimension;
	// The number of bytes of each component. For example, vec3 has three GL_FLOAT components of size 4 each.
	int format;
	// The usage category for this vertex attribute.
	VertexAttributeUsageCategory category;
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
class RenderingPipeline 
{
public:
	RenderingPipeline(PipelineID pipeline_id, RenderingPipelineInfo info);

	~RenderingPipeline();

	const PipelineID& GetInstanceID();

	const std::vector<Shader>& ShaderStages();

	std::size_t IndexOfUniformWithNameAndType(std::string name, ShaderDataType type);

	std::size_t IndexOfVertexAttributeWithNameAndType(std::string name, ShaderDataType type);

	const UniformInfo& UniformInfoAtIndex(std::size_t index);

	const VertexAttributeInfo& VertexAttributeInfoAtIndex(std::size_t index);

private:
	
	PipelineID id_;

	// The uniforms are sorted by appearance order in shaders
	std::vector<UniformInfo> uniforms_;
	// Maps name of uniform to its index in the uniforms_ vector.
	std::unordered_map<std::string, std::size_t> uniform_index_map_;

	// The vertex attributes are sorted by appearance order in shaders
	std::vector<VertexAttributeInfo> vertex_attributes_;
	// Maps name of vertex attribute to its index in the vertex_attributes_ vector.
	std::unordered_map<std::string, std::size_t> vertex_attribute_index_map_;

	std::vector<Shader> shader_stages_;
};