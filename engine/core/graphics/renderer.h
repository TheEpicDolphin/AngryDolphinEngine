#pragma once

#include "mesh_renderable.h"

#include <glm/mat4x4.hpp>

struct RenderableObjectInfo
{
	std::shared_ptr<Material> material;
	std::shared_ptr<Mesh> mesh;
	glm::mat4 model_matrix;
};

class IRenderer {
public:
	virtual void Initialize(int width, int height) = 0;

	virtual void SetRenderableObject(UID id, RenderableObjectInfo info) = 0;

	virtual void UnsetRenderableObject(UID id) = 0;
	
	virtual bool RenderFrame(CameraParams camParams) = 0;

	virtual void Cleanup() = 0;
};