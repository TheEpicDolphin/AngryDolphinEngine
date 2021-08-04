#pragma once

#include <vector>
#include <unordered_map>
#include "shader/shader.h"

typedef UID PipelineID;

enum VertexAttributeType {
	VertexAttributeTypePosition = 0,
	VertexAttributeTypeNormal,
	VertexAttributeTypeTextureCoordinate
};

struct RenderingPipelineDelegate
{
	virtual void PipelineDidDestruct(RenderingPipeline* pipeline) = 0;
};

struct RenderingPipelineInfo
{

};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
class RenderingPipeline 
{
public:
	RenderingPipeline();

	RenderingPipeline(PipelineID pipeline_id, std::vector<Shader> shader_stages);

	const PipelineID& GetInstanceID();

	bool HasUniformWithNameAndType(std::string name, int typeId);

	const std::vector<Shader>& ShaderStages();

private:
	
	struct UniformInfo {
		int typeId;
	};

	PipelineID id_;
	std::vector<VertexAttributeType> vertex_attributes_;
	std::unordered_map<std::string, UniformInfo> uniform_info_map_;
	std::vector<Shader> shader_stages_;
};