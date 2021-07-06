#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include "shader.h"

typedef UID MaterialID;

struct MaterialDelegate 
{
	virtual void MaterialDidDestruct(Material* material) = 0;
};

class Material
{
public:
	Material(MaterialID id, std::shared_ptr<Shader> vertex_shader, std::shared_ptr<Shader> fragment_shader, MaterialDelegate *delegate)
	{
		id_ = id;
		vertex_shader_ = vertex_shader;
		fragment_shader_ = fragment_shader;
		delegate_ = std::make_shared<MaterialDelegate>(delegate);
		// THIS IS TEMPORARY. Later, load shaders at startup only
		program_id_ = LoadShaders("", "");
	}

	~Material() 
	{
		std::shared_ptr<MaterialDelegate> delegate = delegate_.lock();
		if (delegate) {
			delegate->MaterialDidDestruct(this);
		}
	}

	GLuint ProgramID() 
	{
		return program_id_;
	}

	GLuint VertexAttribute() {
		return vertex_attribute_;
	}

	bool IsEqual(Material& otherMaterial) 
	{
		return program_id_ == otherMaterial.program_id_;
	}

	const MaterialID& GetInstanceID() 
	{
		return id_;
	}

	void SetProperty(std::string property_name, bool value)
	{
		bool_property_map[property_name] = value;
	}

	void SetProperty(std::string property_name, int value)
	{
		int_property_map[property_name] = value;
	}

	void SetProperty(std::string property_name, uint32_t value)
	{
		uint_property_map[property_name] = value;
	}

	void SetProperty(std::string property_name, float value)
	{
		float_property_map[property_name] = value;
	}

	void SetProperty(std::string property_name, double value)
	{
		double_property_map[property_name] = value;
	}

private:
	GLuint program_id_;
	GLuint vertex_attribute_ = 0;

	MaterialID id_;
	std::shared_ptr<Shader> vertex_shader_;
	std::shared_ptr<Shader> fragment_shader_;
	
	std::unordered_map<std::string, bool> bool_property_map;
	std::unordered_map<std::string, int> int_property_map;
	std::unordered_map<std::string, uint32_t> uint_property_map;
	std::unordered_map<std::string, float> float_property_map;
	std::unordered_map<std::string, double> double_property_map;

	std::weak_ptr<MaterialDelegate> delegate_;
};