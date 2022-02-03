#include "rapidxml.hpp"
#include "mesh_manager.h"

std::shared_ptr<Mesh> MeshManager::CreateMeshForResourcePath(const char* resource_path_name) {
	std::vector<char> material_asset = ResourceManager::LoadAsset(resource_path_name, "mat");

	rapidxml::xml_document<> material_xml_doc;
	material_xml_doc.parse<0>(material_asset.data());

	const char* rendering_pipeline_path = material_xml_doc.first_node("rendering_pipeline_path")->value();
	std::shared_ptr<RenderingPipeline> rendering_pipeline = RenderingPipelineManager::RenderingPipelineForResourcePath(rendering_pipeline_path);

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

	std::shared_ptr<Material> material = std::make_shared<Material>(++next_mesh_id_, { uniform_settings, rendering_pipeline });
}

std::shared_ptr<Mesh> MeshManager::CreateMesh(MeshInfo info) {
	return std::make_shared<Mesh>(++next_mesh_id_, { info.uniform_settings, info.rendering_pipeline });
}
