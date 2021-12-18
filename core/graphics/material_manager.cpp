
#include "material_manager.h"

#include <experimental/filesystem>

#include "rapidxml.hpp"

namespace fs = std::experimental::filesystem;

static void SetMaterialPropertiesForMaterial(Material *material, )
{
	for () {
		switch (property_type)
		{
		default:
			break;
		}
	}
}

std::shared_ptr<Material> MaterialManager::MaterialForResourcePath(const char* resource_path_name) {
	fs::path resource_path = resource_path_name;
	assert(resource_path.extension() == "mat");
	std::vector<char> material_asset = ResourceManager.LoadAsset(resource_path);
	
	rapidxml::xml_document<> material_xml_doc;
	material_xml_doc.parse<0>(material_asset.data());

	const char* rendering_pipeline_path = material_xml_doc.first_node("rendering_pipeline_path")->value();
	std::shared_ptr<RenderingPipeline> rendering_pipeline = RenderingPipelineManager.MaterialForResourcePath(rendering_pipeline_path);

	std::unordered_map<std::string, UniformValue> uniform_settings;
	rapidxml::xml_node<>* uniform_settings_node = material_xml_doc.first_node("uniform_settings");
	rapidxml::xml_node<>* uniform_setting_node = uniform_settings_node->first_node();
	while (uniform_setting_node != nullptr) {
		std::string uniform_name = uniform_setting_node->first_node("name")->value();
		ShaderDataType shader_data_type = uniform_setting_node->first_node("shader_data_type")->value();
		std::vector<char> uniform_value_data = shader::ValueData(uniform_setting_node->first_node("value")->value());
		uniform_settings[uniform_name] = { 0, shader_data_type , uniform_value_data };
		uniform_setting_node = uniform_setting_node->next_sibling();
	}

	const MaterialID material_id = material_id_generator_.CheckoutNewId();
	std::shared_ptr<Material> material = std::make_shared<Material>(material_id, { uniform_settings, rendering_pipeline }, this);
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(MaterialInfo info) {

}

void MaterialManager::MaterialDidDestruct(Material *material)
{
	material_id_generator_.ReturnId(material->GetInstanceID());
}
