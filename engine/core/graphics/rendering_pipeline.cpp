
#include "rendering_pipeline.h"

RenderingPipeline::RenderingPipeline() {}

RenderingPipeline::RenderingPipeline(PipelineID pipeline_id, std::vector<UniformInfo> uniforms, std::vector<Shader> shader_stages)
{
	id_ = pipeline_id;
	shader_stages_ = shader_stages;
}

const PipelineID& RenderingPipeline::GetInstanceID()
{
	return id_;
}

bool RenderingPipeline::HasUniformWithNameAndType(std::string name, int typeId)
{
	std::unordered_map<std::string, UniformInfo>::iterator iter = uniform_info_map_.find(name);
	return iter != uniform_info_map_.end() && iter->second.typeId == typeId;
}

const std::vector<Shader>& RenderingPipeline::ShaderStages()
{
	return shader_stages_;
}

const std::vector<VertexAttributeInfo>& RenderingPipeline::VertexAttributes()
{
	return vertex_attributes_;
}