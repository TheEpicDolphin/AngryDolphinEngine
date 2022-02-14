
#include "rendering_pipeline_manager.h"

#include "rapidxml.hpp"

#include <core/resource_manager/resource_manager.h>
#include <core/serialize/archive.h>
#include <core/serialize/serdes_utils.h>

#include "shader/shader.h"

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::RenderingPipelineForResourcePath(const char* resource_path)
{
	std::unordered_map<std::string, std::shared_ptr<RenderingPipeline>>::iterator iter = 
		loaded_rendering_pipelines_assets_.find(resource_path);
	if (iter != loaded_rendering_pipelines_assets_.end()) {
		return iter->second;
	}

	std::vector<char> pipeline_asset = resource_manager::ResourceManager::LoadAsset(resource_path);

	rapidxml::xml_document<> xml_doc;
	xml_doc.parse<0>(pipeline_asset.data());
	Archive archive;

	UniformInfo mvp_uniform;
	std::vector<UniformInfo> material_uniforms;
	std::vector<VertexAttributeInfo> vertex_attributes;
	std::vector<shader::Shader> stages;

	rapidxml::xml_node<>* shader_stages_node = xml_doc.first_node("shader_stages");
	rapidxml::xml_node<>* shader_stage_node = shader_stages_node->first_node();
	while (shader_stage_node != nullptr) {
		shader::ShaderStageType stage_type;
		int stage_type_int;
		serialize::DeserializeArithmeticFromString(stage_type_int, shader_stage_node->first_node("type")->value());
		stage_type = (shader::ShaderStageType)stage_type_int;

		if (stage_type == shader::ShaderStageTypeVertex) {
			rapidxml::xml_node<>* mvp_uniform_node = shader_stage_node->first_node("mvp_uniform");
			archive.DeserializeHumanReadable(*mvp_uniform_node, mvp_uniform);

			// Deserialize material uniforms
			rapidxml::xml_node<>* material_uniforms_node = shader_stage_node->first_node("material_uniforms");
			std::vector<UniformInfo> vertex_shader_material_uniforms;
			archive.DeserializeHumanReadable(*material_uniforms_node, vertex_shader_material_uniforms);
			material_uniforms.insert(material_uniforms.end(), vertex_shader_material_uniforms.begin(), vertex_shader_material_uniforms.end());

			// Deserialize vertex attributes
			rapidxml::xml_node<>* vertex_attributes_node = shader_stage_node->first_node("vertex_attributes");
			archive.DeserializeHumanReadable(*vertex_attributes_node, vertex_attributes);
		}
		else if (stage_type == shader::ShaderStageTypeFragment) {
			// Deserialize material uniforms
			rapidxml::xml_node<>* material_uniforms_node = shader_stage_node->first_node("material_uniforms");
			std::vector<UniformInfo> fragment_shader_material_uniforms;
			archive.DeserializeHumanReadable(*material_uniforms_node, fragment_shader_material_uniforms);
			material_uniforms.insert(material_uniforms.end(), fragment_shader_material_uniforms.begin(), fragment_shader_material_uniforms.end());
		}

		char* shader_code_path = shader_stage_node->first_node("code_path")->value();
		std::vector<char> code = resource_manager::ResourceManager::LoadAsset(shader_code_path);
		stages.push_back(shader::Shader(stage_type, code));
		shader_stage_node = shader_stage_node->next_sibling();
	}

	RenderingPipelineInfo rpi = { mvp_uniform, material_uniforms, vertex_attributes, stages };
	std::shared_ptr<RenderingPipeline> pipeline = std::make_shared<RenderingPipeline>(++next_pipeline_id_, rpi);
	loaded_rendering_pipelines_assets_[resource_path] = pipeline;
	return pipeline;
}

std::shared_ptr<RenderingPipeline> RenderingPipelineManager::CreateRenderingPipeline(RenderingPipelineInfo info) {
	return std::make_shared<RenderingPipeline>(++next_pipeline_id_, info.shader_stages);
}

