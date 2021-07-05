#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include "shader.h"

typedef UID MaterialID;

class Material
{
public:
	Material(MaterialID id, Shader vertex_shader, Shader fragment_shader, std::unordered_map<std::string, char*> property_map)
	{
		id_ = id;
		property_map_ = property_map;
		// THIS IS TEMPORARY. Later, load shaders at startup only
		program_id_ = LoadShaders("", "");
	}

	~Material() 
	{

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

	MaterialID GetInstanceID() 
	{
		return id_;
	}

private:
	GLuint program_id_;
	GLuint vertex_attribute_ = 0;

	MaterialID id_;
	Shader vertex_shader_;
	Shader fragment_shader_;
	std::unordered_map<std::string, char*> property_map_;
};