
#include "rendering_pipeline.h"

#include <fstream>

#include <rapidxml/rapidxml.hpp>
#include <core/resource_manager/resource_manager.h>
#include <core/serialize/archive.h>
#include <core/serialize/serdes_utils.h>

#include "shader/shader.h"
#include "shader/shader_vars/shader_var_helpers.h"

RenderingPipeline::RenderingPipeline(RenderingPipelineInfo info)
{
	mvp_uniform_ = info.mvp_uniform;
	material_uniforms_ = info.material_uniforms;
	vertex_attributes_ = info.vertex_attributes;
	shader_stages_ = info.shader_stages;
}

std::shared_ptr<RenderingPipeline> RenderingPipeline::RenderingPipelineForResourcePath(const char* resource_path)
{
	static std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>> loaded_rendering_pipelines_assets;

	std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>>::iterator iter =
		loaded_rendering_pipelines_assets.find(resource_path);
	if (iter != loaded_rendering_pipelines_assets.end()) {
		return iter->second;
	}

	std::vector<char> pipeline_asset = resource_manager::ResourceManager::LoadAsset(resource_path);

	Archive archive;
	rapidxml::xml_document<>* xml_doc = new rapidxml::xml_document<>();
	RenderingPipelineInfo deserialized_rp_info;
	xml_doc->parse<0>(pipeline_asset.data());
	archive.DeserializeHumanReadable(*xml_doc, deserialized_rp_info);
	xml_doc->clear();
	delete xml_doc;

	std::shared_ptr<RenderingPipeline> pipeline = std::make_shared<RenderingPipeline>(deserialized_rp_info);
	loaded_rendering_pipelines_assets[resource_path] = pipeline;
	return pipeline;
}

std::shared_ptr<RenderingPipeline> RenderingPipeline::CreateRenderingPipeline(RenderingPipelineInfo info) {
	return std::make_shared<RenderingPipeline>(info);
}

RenderingPipeline::~RenderingPipeline() {
	lifecycle_events_announcer_.Announce(&PipelineLifecycleEventsListener::PipelineDidDestroy, this);
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
