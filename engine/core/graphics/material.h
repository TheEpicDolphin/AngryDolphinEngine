#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include "rendering_pipeline.h"

typedef UID MaterialID;

struct MaterialDelegate 
{
	virtual void MaterialDidDestruct(Material* material) = 0;
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

	template<typename T>
	void SetUniform(std::string name, T value) 
	{
		rendering_pipeline_->SetUniformValue<T>(name, value);
	}

	template<typename T>
	T GetUniform(std::string name)
	{
		return rendering_pipeline_->GetUniformValue<T>(name);
	}

private:
	GLuint vertex_attribute_ = 0;

	MaterialID id_;

	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	std::weak_ptr<MaterialDelegate> delegate_;
};