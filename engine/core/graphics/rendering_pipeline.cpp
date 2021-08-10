
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

bool RenderingPipeline::HasUniformWithNameAndType(std::string name, int type_id)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = uniform_index_map_.find(name);
	if (iter == uniform_index_map_.end()) {
		return false;
	}

	const UniformInfo uniform = uniforms_[iter->second];
	return uniform.type_id == type_id;
}

bool RenderingPipeline::HasVertexAttributeWithNameAndType(std::string name, int type_id)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = vertex_attribute_index_map_.find(name);
	if (iter == vertex_attribute_index_map_.end()) {
		return false;
	}

	const VertexAttributeInfo vertex_attribute = vertex_attributes_[iter->second];
	return vertex_attribute.type_id == type_id;
}

const std::vector<Shader>& RenderingPipeline::ShaderStages()
{
	return shader_stages_;
}

const std::vector<UniformInfo>& RenderingPipeline::Uniforms() {
	return uniforms_;
}

const std::vector<VertexAttributeInfo>& RenderingPipeline::VertexAttributes()
{
	return vertex_attributes_;
}
