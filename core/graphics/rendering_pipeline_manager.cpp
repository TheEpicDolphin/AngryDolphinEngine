
#include "rendering_pipeline_manager.h"

#include "rapidxml.hpp"

#include <core/resources/resource_manager.h>

#include "shader/shader.h"

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::RenderingPipelineForResourcePath(const char* resource_path)
{
	std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>>::iterator iter = 
		loaded_rendering_pipelines_assets_.find(resource_path);
	if (iter != loaded_rendering_pipelines_assets_.end()) {
		return iter->second;
	}

	std::vector<char> pipeline_asset = resources::ResourceManager::LoadAsset(resource_path, "rp");

	rapidxml::xml_document<> xml_doc;
	xml_doc.parse<0>(pipeline_asset.data());

	UniformInfo mvp_uniform;
	std::vector<UniformInfo> material_uniforms;
	std::vector<VertexAttributeInfo> vertex_attributes;
	std::vector<shader::Shader> stages;

	rapidxml::xml_node<>* shader_stages_node = xml_doc.first_node("shader_stages");
	rapidxml::xml_node<>* shader_stage_node = shader_stages_node->first_node();
	while (shader_stage_node != nullptr) {
		shader::ShaderStageType stage_type = shader_stage_node->first_node("type")->value();
		if (stage_type == shader::ShaderStageTypeVertex) {
			rapidxml::xml_node<>* mvp_uniform_node = shader_stage_node->first_node("mvp_uniform");

			// Parse exposed material uniforms
			rapidxml::xml_node<>* material_uniforms_node = shader_stage_node->first_node("material_uniforms");

			// Parse exposed vertex attributes
			rapidxml::xml_node<>* vertex_attributes_node = shader_stage_node->first_node("vertex_attributes")->value();

		}
		else if (stage_type == shader::ShaderStageTypeFragment) {
			// Parse exposed material uniforms
			rapidxml::xml_node<>* material_uniforms_node = shader_stage_node->first_node("material_uniforms");

		}

		char* shader_code_path = shader_stage_node->first_node("code_path")->value();
		std::vector<char> code = resources::ResourceManager::LoadAsset(shader_code_path);
		stages.push_back(shader::Shader(stage_type, code));
		shader_stage_node = shader_stage_node->next_sibling();
	}

	std::shared_ptr<RenderingPipeline> pipeline = std::make_shared<RenderingPipeline>(++next_pipeline_id_, { mvp_uniform, material_uniforms, vertex_attributes, stages });
	loaded_rendering_pipelines_assets_[resource_path] = pipeline;
	return pipeline;
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::CreateRenderingPipeline(RenderingPipelineInfo info) {
	return std::make_shared<RenderingPipeline>(++next_pipeline_id_, info.shader_stages);
}

