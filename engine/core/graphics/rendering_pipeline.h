#pragma once

#include <vector>
#include <unordered_map>
#include "shader/shader.h"

typedef UID PipelineID;

struct RenderingPipelineDelegate
{
	virtual void PipelineDidDestruct(RenderingPipeline* pipeline) = 0;
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
class RenderingPipeline 
{
public:
	RenderingPipeline();

	RenderingPipeline(PipelineID pipeline_id, std::vector<Shader> shader_stages);

	const PipelineID& GetInstanceID();

	bool HasUniformWithNameAndType(std::string name, int typeId);

private:
	
	struct UniformInfo {
		int typeId;
	};

	PipelineID id_;
	std::unordered_map<std::string, UniformInfo> uniform_info_map_;
	std::vector<Shader> shader_stages_;
	std::unordered_map<ShaderStage, std::size_t> shader_stage_map_;
};