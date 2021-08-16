
#include "rendering_pipeline.h"

#include "shader/shader_vars/shader_var_helpers.h"

RenderingPipeline::RenderingPipeline() {}

RenderingPipeline::RenderingPipeline(PipelineID pipeline_id, RenderingPipelineInfo info)
{
	id_ = pipeline_id;

	shader_stages_ = shader_stages;
}

const PipelineID& RenderingPipeline::GetInstanceID()
{
	return id_;
}

std::size_t RenderingPipeline::IndexOfUniformWithNameAndType(std::string name, ShaderDataType type)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = uniform_index_map_.find(name);
	if (iter == uniform_index_map_.end()) {
		return shader::npos;
	}

	const UniformInfo uniform = uniforms_[iter->second];
	if (uniform.type != type) {
		return shader::npos;
	}

	return iter->second;
}

std::size_t RenderingPipeline::IndexOfVertexAttributeWithNameAndType(std::string name, ShaderDataType type)
{
	std::unordered_map<std::string, std::size_t>::iterator iter = vertex_attribute_index_map_.find(name);
	if (iter == vertex_attribute_index_map_.end()) {
		return shader::npos;
	}

	const VertexAttributeInfo vertex_attribute = vertex_attributes_[iter->second];
	if (vertex_attribute.type != type) {
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
