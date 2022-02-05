
#include "material.h"

Material::Material(MaterialID id, MaterialInfo info)
{
	id_ = id;
	rendering_pipeline_ = info.rendering_pipeline;

	uniform_values_ = {};
	for (UniformInfo uniform_info : rendering_pipeline_->Uniforms()) {
		const std::size_t index = uniform_values_.size();
		uniform_values_.push_back({
			uniform_info.data_type,
			std::vector<char>()
		});
		uniform_value_name_map_[uniform_info.name] = index;
		switch (vertex_attribute_info.category)
		{
		case UniformUsageCategoryColor:
			color_uniform_index_ = index;
			break;
		case UniformUsageCategoryCustom:
			// Do nothing
			break;
		}
	}
}

Material::~Material()
{
	lifecycle_events_announcer_.Announce(&MaterialLifecycleEventsListener::MaterialDidDestroy, this->GetInstanceID());
}

const MaterialID& Material::GetInstanceID()
{
	return id_;
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