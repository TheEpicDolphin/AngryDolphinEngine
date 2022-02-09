
#include "material.h"

Material::Material(MaterialID id, MaterialInfo info)
{
	id_ = id;
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

Material::~Material()
{
	lifecycle_events_announcer_.Announce(&MaterialLifecycleEventsListener::MaterialDidDestroy, this->GetInstanceID());
}

const MaterialID& Material::GetInstanceID()
{
	return id_;
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