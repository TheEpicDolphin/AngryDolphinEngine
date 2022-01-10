#include "rendering_pipeline_manager.h"

#include "rapidxml.hpp"

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::CreateRenderingPipeline(RenderingPipelineInfo info) {
	const PipelineID pipeline_id = pipeline_id_generator_->CheckoutNewId();
	return std::make_shared<RenderingPipeline>(pipeline_id, info.shader_stages, this);
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::RenderingPipelineForResourcePath(const char* resource_path)
{
	std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>>::iterator iter = 
		loaded_rendering_pipelines_assets_.find(resource_path);
	if (iter != loaded_rendering_pipelines_assets_.end()) {
		return iter->second;
	}

	std::vector<char> pipeline_asset = ResourceManager::LoadAsset(resource_path, "rp");

	rapidxml::xml_document<> xml_doc;
	xml_doc.parse<0>(pipeline_asset.data());

	const char* rendering_pipeline_path = material_xml_doc.first_node("rendering_pipeline_path")->value();
	std::shared_ptr<RenderingPipeline> rendering_pipeline = RenderingPipelineManager::RenderingPipelineForResourcePath(rendering_pipeline_path);

	std::vector<Shader> stages;
	rapidxml::xml_node<>* shader_stages_node = xml_doc.first_node("shader_stages");
	rapidxml::xml_node<>* shader_stage_node = shader_stages_node->first_node();
	while (shader_stage_node != nullptr) {
		ShaderStageType stage_type = shader_stage_node->first_node("type")->value();
		std::vector<char> code = shader_stage_node->first_node("code")->value();
		stages.push_back(Shader(stage_type, code));
		shader_stage_node = shader_stage_node->next_sibling();
	}

	const PipelineID pipeline_id = pipeline_id_generator_->CheckoutNewId();
	std::shared_ptr<RenderingPipeline> pipeline = std::make_shared<RenderingPipeline>(pipeline_id, { stages }, this);
	loaded_rendering_pipelines_assets_[resource_path] = pipeline;
	return pipeline;
}

void RenderingPipelineManager::PipelineDidDestruct(RenderingPipeline* pipeline)
{
	pipeline_id_generator_->ReturnId(pipeline->GetInstanceID());
}


