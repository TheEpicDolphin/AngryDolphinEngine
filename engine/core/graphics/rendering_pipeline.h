#pragma once

#include <vector>
#include <unordered_map>
#include "shader/shader.h"

typedef UID PipelineID;

struct RenderingPipelineDelegate
{
	virtual void PipelineDidDestruct(RenderingPipeline* pipeline) = 0;
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
class RenderingPipeline 
{
public:
	RenderingPipeline();

	RenderingPipeline(PipelineID pipeline_id, std::vector<Shader> shader_stages);

	const PipelineID& GetInstanceID();

	template<typename T>
	void SetUniformValue(std::string name, T& value) 
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
		shader_var->SetValue(value);
	}

	template<typename T>
	const T& GetUniformValue(std::string name) 
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
	PipelineID id_;
	std::unordered_map<std::string, Uniform> uniform_map_;
	std::vector<Shader> shader_stages_;
	std::unordered_map<ShaderStage, std::size_t> shader_stage_map_;
};