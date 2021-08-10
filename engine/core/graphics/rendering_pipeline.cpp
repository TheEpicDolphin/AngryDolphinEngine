
#include "rendering_pipeline.h"

#include "shader/shader_vars/shader_var_helpers.h"

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

std::size_t RenderingPipeline::IndexOfUniformWithNameAndType(std::string name, int type_id)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = uniform_index_map_.find(name);
	if (iter == uniform_index_map_.end()) {
		return shader::npos;
	}

	const UniformInfo uniform = uniforms_[iter->second];
	if (uniform.type_id != type_id) {
		return shader::npos;
	}

	return iter->second;
}

std::size_t RenderingPipeline::IndexOfVertexAttributeWithNameAndType(std::string name, int type_id)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = vertex_attribute_index_map_.find(name);
	if (iter == vertex_attribute_index_map_.end()) {
		return shader::npos;
	}

	const VertexAttributeInfo vertex_attribute = vertex_attributes_[iter->second];
	if (vertex_attribute.type_id != type_id) {
		return shader::npos;
	}

	return iter->second;
}

const std::vector<Shader>& RenderingPipeline::ShaderStages()
{
	return shader_stages_;
}

const UniformInfo& RenderingPipeline::UniformInfoAtIndex(std::size_t index)
{
	return uniforms_[index];
}

const VertexAttributeInfo& RenderingPipeline::VertexAttributeInfoAtIndex(std::size_t index)
{
	return vertex_attributes_[index];
}
