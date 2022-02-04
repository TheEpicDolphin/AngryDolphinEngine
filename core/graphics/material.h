#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include <core/utils/uid_generator.h>

#include "shader/shader_vars/shader_var_helpers.h"
#include "rendering_pipeline.h"

typedef std::uint32_t MaterialID;

struct UniformValue {
	std::size_t uniform_index;
	ShaderDataType type;
	std::vector<char> data;
	bool is_dirty;
};

struct MaterialInfo
{
	std::unordered_map<std::string, UniformValue> uniform_settings;
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
};

struct MaterialLifecycleEventsListener {

	virtual void MaterialDidDestroy(MaterialID materialId) = 0;

	virtual void MaterialUniformDidChange(Material* material, std::size_t uniform_index) = 0;
};

class Material
{
public:
	Material(MaterialID id, MaterialInfo info)
	{
		id_ = id;
		rendering_pipeline_ = info.rendering_pipeline;
	}

	~Material() 
	{}

	const MaterialID& GetInstanceID()
	{
		return id_;
	}

	bool IsEqual(Material& other_material) 
	{
		return id_ == other_material.id_;
	}

	template<typename T>
	void SetUniform(std::string name, T value) 
	{
		const ShaderDataType type = shader::TypeID(value);
		const std::vector<char> value_data = shader::ValueData(value);
		// Check if rendering pipeline actually has a uniform with this name and type.
		const std::size_t index = rendering_pipeline_->IndexOfUniformWithNameAndType(name, type);
		if (index != shader::index_not_found) {
			uniform_value_index_map_[name] = uniform_values_.size();
			uniform_values_.push_back({ index, type, value_data });
			
			lifecycle_events_listener_->MaterialUniformDidChange(this, index);
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	template<typename T>
	void SetUniformArray(std::string name, T value_array[])
	{
		const ShaderDataType type = shader::TypeID(value_array[0]);
		const std::vector<char> value_data = shader::ValueArrayData(value_array);
		// Check if rendering pipeline actually has a uniform with this name and type.
		const std::size_t index = rendering_pipeline_->IndexOfUniformWithNameAndType(name, type);
		if (index != shader::index_not_found) {
			uniform_value_index_map_[name] = uniform_values_.size();
			uniform_values_.push_back({ index, type, value_data });

			lifecycle_events_listener_->MaterialUniformDidChange(this, index);
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	template<typename T>
	void GetUniform(std::string name, T* value) 
	{
		std::unordered_map<std::string, std::size_t>::iterator iter = uniform_value_index_map_.find(name);
		if (iter == uniform_value_index_map_.end()) {
			// print warning that uniform with this name does not exist or has not been assigned for this material.
		}

		UniformValue& uniform_value = uniform_values_[iter->second];
		// Check that the found uniform has the expected type
		if (shader::TypeId(*value) == uniform_value.type) {
			shader::MakeValue(value, uniform_value.data);
		}
		else {
			// print warning that uniform with this type does not exist for this material
		}
	}

	const std::vector<UniformValue>& UniformValues() 
	{
		return uniform_values_;
	}

	const std::shared_ptr<RenderingPipeline>& GetPipeline() {
		return rendering_pipeline_;
	}

private:
	MaterialID id_;

	std::vector<UniformValue> uniform_values_;
	std::unordered_map<std::string, std::size_t> uniform_value_index_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;

	MaterialLifecycleEventsListener* lifecycle_events_listener_;
};