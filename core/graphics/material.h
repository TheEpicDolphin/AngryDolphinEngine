#pragma once

#include <iostream>
#include <glm/vec4.hpp>
#include <vector>

#include <core/utils/event_announcer.h>

#include "shader/shader_vars/shader_data_type.h"
#include "shader/shader_vars/shader_var_helpers.h"
#include "rendering_pipeline.h"

struct UniformValue {
	shader::ShaderDataType data_type;
	std::vector<char> data;
	int array_length;
};

struct MaterialInfo
{
	//std::unordered_map<std::string, std::vector<char>> initial_uniform_settings;
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
};

class Material;

struct MaterialLifecycleEventsListener {

	virtual void MaterialUniformDidChange(Material* material, std::size_t uniform_index) = 0;
	//virtual void MaterialTextureDidChange(Material* material, Texture texture) = 0;
	virtual void MaterialDidDestroy(Material* material) = 0;
};

class Material
{
public:

	//static std::shared_ptr<Material> CreateMaterialForResourcePath(const char* resource_path);

	static std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

	~Material();

	void SetColor(glm::vec4 color);

	const glm::vec4& GetColor();

	const std::vector<UniformValue>& UniformValues();

	const std::shared_ptr<RenderingPipeline>& GetPipeline();

	void AddLifecycleEventsListener(MaterialLifecycleEventsListener* listener);

	void RemoveLifecycleEventsListener(MaterialLifecycleEventsListener* listener);

	template<typename T>
	void SetUniform(std::string name, T value)
	{
		std::unordered_map<std::string, std::size_t>::iterator iter = uniform_value_name_map_.find(name);
		if (iter == uniform_value_name_map_.end()) {
			// TODO: print warning that a vertex attribute with this name does not exist for this mesh/pipeline.
			return;
		}
		const std::size_t uniform_index = iter->second;

		UniformValue& uniform_value = uniform_values_[uniform_index];
		const shader::ShaderDataType data_type = shader::TypeID(value);
		if (uniform_value.data_type != data_type) {
			// TODO: print warning that the vertex attribute with this name does not have the inputted type.
			return;
		}

		const std::vector<char> value_data = shader::SerializeValueToCompactBytes(value);
		uniform_value.data = value_data;

		lifecycle_events_announcer_.Announce(&MaterialLifecycleEventsListener::MaterialUniformDidChange, this, uniform_index);
	}

	// TODO: SetUniformArray function

	template<typename T>
	void GetUniform(std::string name, T* value)
	{
		std::unordered_map<std::string, std::size_t>::iterator iter = uniform_value_name_map_.find(name);
		if (iter == uniform_value_index_map_.end()) {
			// print warning that uniform with this name does not exist or has not been assigned for this material.
		}

		UniformValue& uniform_value = uniform_values_[iter->second];
		// Check that the found uniform has the expected type
		if (shader::TypeId(*value) == uniform_value.type) {
			shader::DeserializeValueFromCompactBytes(value, uniform_value.data);
		}
		else {
			// print warning that uniform with this type does not exist for this material
		}
	}

private:
	// Indices to commonly used material uniforms.
	int color_uniform_index_ = -1;

	std::vector<UniformValue> uniform_values_;
	std::unordered_map<std::string, std::size_t> uniform_value_name_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;

	EventAnnouncer<MaterialLifecycleEventsListener> lifecycle_events_announcer_;

	//Texture texture_;

	Material(MaterialInfo info);

	template<typename T>
	void SetUniformWithCachedIndex(int cached_uniform_index, T value) {
		uniform_values_[cached_uniform_index].data = shader::SerializeValueToCompactBytes(value);
		lifecycle_events_announcer_.Announce(&MaterialLifecycleEventsListener::MaterialUniformDidChange, this, cached_uniform_index);
	}

	template<typename T>
	void GetUniformWithCachedIndex(int cached_uniform_index, const T* value) {
		UniformValue& uniform_value = uniform_values_[cached_uniform_index];
		value = reinterpret_cast<T*>(uniform_value.data.data());
	}
};