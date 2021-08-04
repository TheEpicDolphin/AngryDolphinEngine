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

struct MaterialInfo
{
	std::unordered_map<std::string, ShaderVarValue> uniform_settings;
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

	GLuint VertexAttribute() {
		return vertex_attribute_;
	}

	bool IsEqual(Material& otherMaterial) 
	{
		return id_ == otherMaterial.id_;
	}

	const MaterialID& GetInstanceID() 
	{
		return id_;
	}

	const PipelineID& GetPipelineID() 
	{
		return rendering_pipeline_->GetInstanceID();
	}

	template<typename T>
	void SetUniform(std::string name, T value) 
	{
		const int type_id = shader::TypeID(value);
		const std::vector<char> value_data = shader::ValueData(value);
		// Check if rendering pipeline actually has a uniform with this name and type.
		if (rendering_pipeline_->HasUniformWithNameAndType(name, type_id)) {
			uniform_value_map_[name] = { type_id, value_data };
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	template<typename T>
	void GetUniform(std::string name, T* value) 
	{
		std::unordered_map<std::string, ShaderVarValue>::iterator iter = uniform_value_map_.find(name);
		// Check that the material has a uniform with this name and type.
		if (iter != uniform_map_.end() && shader::TypeId(*value) == iter->second.type_id) {
			shader::MakeValue(value, iter->second.data);
		}
		else {
			// print warning that uniform with this name and/or type does not exist for this material
		}
	}

private:

	struct ShaderVarValue {
		int type_id;
		std::vector<char> data;
	};

	MaterialID id_;

	std::unordered_map<std::string, ShaderVarValue> uniform_value_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	std::weak_ptr<MaterialDelegate> delegate_;
};