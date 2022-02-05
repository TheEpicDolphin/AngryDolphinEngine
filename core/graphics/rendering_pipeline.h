#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <core/utils/event_announcer.h>

#include "shader/shader.h"

typedef std::uint32_t PipelineID;

enum VertexAttributeUsageCategory
{
	VertexAttributeUsageCategoryPosition = 0,
	VertexAttributeUsageCategoryNormal,
	VertexAttributeUsageCategoryTexCoord0,
	VertexAttributeUsageCategoryBoneWeight,
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
	ShaderDataType data_type;
	// Location in the shader.
	int location;
	// Array length. It is 1 for non-arrays.
	int array_length;
};

struct VertexAttributeInfo {
	// Name of this vertex attribute.
	std::string name;
	// Data type.
	ShaderDataType data_type;
	// Location in the shader.
	int location;
	// Number of components in the data type. For example, vec3 has a dimension of 3.
	int dimension;
	// The number of bytes of each component. For example, vec3 has three GL_FLOAT components of size 4 each.
	int format;
	// The usage category for this vertex attribute.
	VertexAttributeUsageCategory category;
};

struct PipelineLifecycleEventsListener {
	virtual void PipelineDidDestroy(PipelineID pipeline_id) = 0;
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
// It is an immutable class
class RenderingPipeline 
{
public:
	RenderingPipeline(PipelineID pipeline_id, RenderingPipelineInfo info);

	~RenderingPipeline();

	const PipelineID& GetInstanceID();

	const std::vector<Shader>& ShaderStages();

	std::size_t IndexOfUniformWithNameAndType(std::string name, ShaderDataType type);

	const UniformInfo& UniformInfoAtIndex(std::size_t index);

	const VertexAttributeInfo& VertexAttributeInfoAtIndex(std::size_t index);

	const std::vector<VertexAttributeInfo>& VertexAttributes();

	void RenderingPipeline::AddLifecycleEventsListener(PipelineLifecycleEventsListener* listener);

	void RenderingPipeline::RemoveLifecycleEventsListener(PipelineLifecycleEventsListener* listener);

private:
	
	const PipelineID id_;

	// The uniforms are sorted by appearance order in shaders
	const std::vector<UniformInfo> uniforms_;
	// Maps name of uniform to its index in the uniforms_ vector.
	const std::unordered_map<std::string, std::size_t> uniform_index_map_;

	// The vertex attributes are sorted by appearance order in shaders
	const std::vector<VertexAttributeInfo> vertex_attributes_;
	// Maps name of vertex attribute to its index in the vertex_attributes_ vector.
	const std::unordered_map<std::string, std::size_t> vertex_attribute_index_map_;

	const std::vector<Shader> shader_stages_;

	EventAnnouncer<PipelineLifecycleEventsListener> lifecycle_events_announcer_;
};