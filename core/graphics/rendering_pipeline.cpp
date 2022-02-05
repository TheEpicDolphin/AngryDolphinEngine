
#include "rendering_pipeline.h"

#include "shader/shader_vars/shader_var_helpers.h"

#include "rendering_pipeline_manager.h"

RenderingPipeline::RenderingPipeline(PipelineID pipeline_id, RenderingPipelineInfo info)
{
	id_ = pipeline_id;
	shader_stages_ = info.shader_stages;
}

RenderingPipeline::~RenderingPipeline() {
	lifecycle_events_announcer_.Announce(&PipelineLifecycleEventsListener::PipelineDidDestroy, this->GetInstanceID());
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

const std::vector<VertexAttributeInfo>& RenderingPipeline::VertexAttributes() {
	return vertex_attributes_;
}

void RenderingPipeline::AddLifecycleEventsListener(PipelineLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.AddListener(listener);
}

void RenderingPipeline::RemoveLifecycleEventsListener(PipelineLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.RemoveListener(listener);
}
