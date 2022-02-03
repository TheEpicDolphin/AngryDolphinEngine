
#include "material_manager.h"

#include "rapidxml.hpp"
#include "rendering_pipeline_manager.h"

static void SetMaterialPropertiesForMaterial(Material *material)
{
	/*
	for () {
		switch (property_type)
		{
		default:
			break;
		}
	}
	*/
}

std::shared_ptr<Material> MaterialManager::CreateMaterialForResourcePath(const char* resource_path_name) {
	std::vector<char> material_asset = ResourceManager::LoadAsset(resource_path_name, "mat");
	
	rapidxml::xml_document<> xml_doc;
	xml_doc.parse<0>(material_asset.data());

	const char* rendering_pipeline_path = xml_doc.first_node("rendering_pipeline_path")->value();
	std::shared_ptr<RenderingPipeline> rendering_pipeline = RenderingPipelineManager::RenderingPipelineForResourcePath(rendering_pipeline_path);

	std::unordered_map<std::string, UniformValue> uniform_settings;
	rapidxml::xml_node<>* uniform_settings_node = xml_doc.first_node("uniform_settings");
	rapidxml::xml_node<>* uniform_setting_node = uniform_settings_node->first_node();
	while (uniform_setting_node != nullptr) {
		std::string uniform_name = uniform_setting_node->first_node("name")->value();
		ShaderDataType shader_data_type = uniform_setting_node->first_node("shader_data_type")->value();
		std::vector<char> uniform_value_data = shader::ValueData(uniform_setting_node->first_node("value")->value());
		uniform_settings[uniform_name] = { 0, shader_data_type , uniform_value_data };
		uniform_setting_node = uniform_setting_node->next_sibling();
	}

	std::shared_ptr<Material> material = std::make_shared<Material>(++next_material_id_, { uniform_settings, rendering_pipeline });
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(MaterialInfo info) {
	return std::make_shared<Material>(++next_material_id_, info);
}
