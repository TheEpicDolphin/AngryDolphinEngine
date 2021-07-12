#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include "shader/shader.h"
#include "shader/uniform.h"

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

	template<typename T>
	void SetUniform(std::string name, T value) 
	{
		std::unordered_map<std::string, Uniform>::iterator iter = uniform_map_.find(name);
		if (iter == uniform_map_.end()) {
			// TODO: print warning that uniform with name %name% does not exist on this material
			return;
		}
		const Uniform& uniform = iter->second;
		if (uniform.shader_var.type != ShaderVar<T>::type) {
			// TODO: print warning "you are attempting to set uniform with name %name% of type %uniform.shader_var.type% to value of type %ShaderVar<T>::type%"
			return;
		}

		const ShaderVar<T> *shader_var = static_cast<ShaderVar<T> *>(&uniform.shader_var);
		shader_var->SetValue(value);
	}

	template<typename T>
	T GetUniform(std::string name)
	{
		std::unordered_map<std::string, Uniform>::iterator iter = uniform_map_.find(name);
		if (iter == uniform_map_.end()) {
			// TODO: print warning that uniform with name %name% does not exist on this material
			return;
		}
		const Uniform& uniform = iter->second;
		if (uniform.shader_var.type != ShaderVar<T>::type) {
			// TODO: print warning "you are attempting to set uniform with name %name% of type %uniform.shader_var.type% to value of type %ShaderVar<T>::type%"
			return;
		}

		const ShaderVar<T>* shader_var = static_cast<ShaderVar<T> *>(&uniform.shader_var);
		return shader_var->GetValue();
	}

private:
	GLuint program_id_;
	GLuint vertex_attribute_ = 0;

	MaterialID id_;
	std::shared_ptr<Shader> vertex_shader_;
	std::shared_ptr<Shader> fragment_shader_;
	
	std::unordered_map<std::string, Uniform> uniform_map_;

	std::weak_ptr<MaterialDelegate> delegate_;
};