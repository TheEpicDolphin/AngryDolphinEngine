
#include "material.h"

/*
#include "rapidxml.hpp"

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
	return std::make_shared<Material>(++next_material_id_, { uniform_settings, rendering_pipeline });
}
*/

Material::Material(MaterialInfo info)
{
	rendering_pipeline_ = info.rendering_pipeline;

	uniform_values_ = {};
	for (const UniformInfo& uniform_info : rendering_pipeline_->MaterialUniforms()) {
		const std::size_t index = uniform_values_.size();
		uniform_values_.push_back({
			uniform_info.data_type,
			std::vector<char>()
		});
		uniform_value_name_map_[uniform_info.name] = index;
		switch (uniform_info.category)
		{
		case UniformUsageCategoryColor:
			color_uniform_index_ = (int)index;
			break;
		case UniformUsageCategoryCustom:
			// Do nothing
			break;
		}
	}

	// TODO: Set initial material uniform values.
}

std::shared_ptr<Material> Material::CreateMaterial(MaterialInfo info) {
	return std::make_shared<Material>(info);
}

Material::~Material()
{
	lifecycle_events_announcer_.Announce(&MaterialLifecycleEventsListener::MaterialDidDestroy, this);
}

void Material::SetColor(glm::vec4 color) {
	SetUniformWithCachedIndex(color_uniform_index_, color);
}

const glm::vec4& Material::GetColor() {
	glm::vec4 color;
	GetUniformWithCachedIndex(color_uniform_index_, &color);
	return color;
}

const std::vector<UniformValue>& Material::UniformValues()
{
	return uniform_values_;
}

const std::shared_ptr<RenderingPipeline>& Material::GetPipeline() {
	return rendering_pipeline_;
}

void Material::AddLifecycleEventsListener(MaterialLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.AddListener(listener);
}

void Material::RemoveLifecycleEventsListener(MaterialLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.RemoveListener(listener);
}