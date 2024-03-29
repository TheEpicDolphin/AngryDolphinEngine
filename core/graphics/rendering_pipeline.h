#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <core/utils/event_announcer.h>
//#include <core/serialize/serializable.h>

#include "shader/shader.h"
#include "shader/shader_vars/shader_data_type.h"

enum class UniformUsageCategory
{
	Color = 0,
	MVP,
	Custom,
};

struct UniformInfo {
	// Name of this uniform.
	std::string name;
	// Data type. In the case of an array, this is the type of each element.
	shader::ShaderDataType data_type;
	// Location in the shader.
	int location;
	// Array length. It is 1 for non-arrays.
	int array_length;
	// The usage category for this uniform.
	UniformUsageCategory category;

	//SERIALIZE_MEMBERS(name, data_type, location, array_length, category)
};

enum class VertexAttributeUsageCategory
{
	Position = 0,
	Normal,
	TexCoord0,
	BoneWeight,
	BoneIndices,
	Custom,
};

struct VertexAttributeInfo {
	// Name of this vertex attribute.
	std::string name;
	// Data type.
	shader::ShaderDataType data_type;
	// Location in the shader.
	int location;
	// TODO: Get rid of dimension and format. They can be calculated from data_type.
	// Number of components in the data type. For example, vec3 has a dimension of 3.
	int dimension;
	// The number of bytes of each component. For example, vec3 has three GL_FLOAT components of size 4 each.
	int format;
	// The usage category for this vertex attribute.
	VertexAttributeUsageCategory category;

	//SERIALIZE_MEMBERS(name, data_type, location, dimension, format, category)
};

class RenderingPipeline;

struct PipelineLifecycleEventsListener {
	virtual void PipelineDidDestroy(RenderingPipeline* pipeline) = 0;
};

struct RenderingPipelineInfo
{
	UniformInfo mvp_uniform;
	std::vector<UniformInfo> material_uniforms;
	std::vector<VertexAttributeInfo> vertex_attributes;
	std::vector<shader::Shader> shader_stages;

	//SERIALIZE_MEMBERS(mvp_uniform, material_uniforms, vertex_attributes, shader_stages)
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
// It is an immutable class
class RenderingPipeline 
{
public:
	RenderingPipeline() {}

	RenderingPipeline(RenderingPipelineInfo info);

	static std::shared_ptr<RenderingPipeline> RenderingPipelineForResourcePath(const char* resource_path);

	static std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	~RenderingPipeline();

	const std::vector<shader::Shader>& ShaderStages();

	const UniformInfo& MVPUniform();

	const std::vector<UniformInfo>& MaterialUniforms();

	const VertexAttributeInfo& VertexAttributeInfoAtIndex(std::size_t index);

	const std::vector<VertexAttributeInfo>& VertexAttributes();

	void RenderingPipeline::AddLifecycleEventsListener(PipelineLifecycleEventsListener* listener);

	void RenderingPipeline::RemoveLifecycleEventsListener(PipelineLifecycleEventsListener* listener);

private:
	UniformInfo mvp_uniform_;

	//const UniformInfo bones_uniform_;

	// The uniforms are sorted by location in shaders
	std::vector<UniformInfo> material_uniforms_;

	// The vertex attributes are sorted by appearance order in shaders
	std::vector<VertexAttributeInfo> vertex_attributes_;

	std::vector<shader::Shader> shader_stages_;

	EventAnnouncer<PipelineLifecycleEventsListener> lifecycle_events_announcer_;
};