#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include "shader/shader_vars/shader_var_helpers.h"
#include "rendering_pipeline.h"

typedef UID MaterialID;

struct MaterialDelegate 
{
	virtual void MaterialDidDestruct(Material* material) = 0;
};

struct UniformValue {
	std::size_t uniform_index;
	int type_id;
	std::vector<char> data;
};

struct MaterialInfo
{
	std::unordered_map<std::string, UniformValue> uniform_settings;
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
};

class Material
{
public:
	Material(MaterialID id, std::shared_ptr<RenderingPipeline> rendering_pipeline, MaterialDelegate *delegate)
	{
		id_ = id;
		rendering_pipeline_ = rendering_pipeline;
		delegate_ = std::make_shared<MaterialDelegate>(delegate);
	}

	~Material() 
	{
		std::shared_ptr<MaterialDelegate> delegate = delegate_.lock();
		if (delegate) {
			delegate->MaterialDidDestruct(this);
		}
	}

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
		const int type_id = shader::TypeID(value);
		const std::vector<char> value_data = shader::ValueData(value);
		// Check if rendering pipeline actually has a uniform with this name and type.
		const std::size_t index = rendering_pipeline_->IndexOfUniformWithNameAndType(name, type_id);
		if (index != shader::index_not_found) {
			uniform_value_map_[name] = { index, type_id, value_data };
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	template<typename T>
	void GetUniform(std::string name, T* value) 
	{
		std::unordered_map<std::string, UniformValue>::iterator iter = uniform_value_map_.find(name);
		// Check that the material has a uniform with this name and type.
		if (iter != uniform_value_map_.end() && shader::TypeId(*value) == iter->second.type_id) {
			shader::MakeValue(value, iter->second.data);
		}
		else {
			// print warning that uniform with this name and/or type does not exist for this material
		}
	}

	std::vector<char> GetUniformData(std::string name)
	{
		std::unordered_map<std::string, UniformValue>::iterator iter = uniform_value_map_.find(name);
		// Check that the material has a uniform with this name and type.
		if (iter != uniform_value_map_.end()) {
			return iter->second.data;
		}
		else {
			// print warning that uniform with this name and/or type does not exist for this material
			return {};
		}
	}

	const std::shared_ptr<RenderingPipeline>& GetPipeline() {
		return rendering_pipeline_;
	}

private:
	MaterialID id_;

	std::unordered_map<std::string, UniformValue> uniform_value_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	std::weak_ptr<MaterialDelegate> delegate_;
};