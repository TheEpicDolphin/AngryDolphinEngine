
#include "rendering_pipeline.h"

#include "shader/shader_vars/shader_var_helpers.h"

#include "rendering_pipeline_manager.h"

RenderingPipeline::RenderingPipeline(PipelineID pipeline_id, RenderingPipelineInfo info)
{
	id_ = pipeline_id;
	mvp_uniform_ = info.mvp_uniform;
	material_uniforms_ = info.material_uniforms;
	vertex_attributes_ = info.vertex_attributes;
	shader_stages_ = info.shader_stages;
}

RenderingPipeline::~RenderingPipeline() {
	lifecycle_events_announcer_.Announce(&PipelineLifecycleEventsListener::PipelineDidDestroy, this->GetInstanceID());
}

const PipelineID& RenderingPipeline::GetInstanceID()
{
	return id_;
}

const std::vector<shader::Shader>& RenderingPipeline::ShaderStages()
{
	return shader_stages_;
}

const UniformInfo& RenderingPipeline::MVPUniform() {
	return mvp_uniform_;
}

const std::vector<UniformInfo>& RenderingPipeline::MaterialUniforms() {
	return material_uniforms_;
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
