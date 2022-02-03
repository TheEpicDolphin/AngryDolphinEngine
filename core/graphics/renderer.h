#pragma once

#include "mesh_renderable.h"
#include "rendering_pipeline.h"

#include <glm/mat4x4.hpp>

struct RenderableObject
{
	Mesh* mesh;
	glm::mat4 model_matrix;
	std::vector<glm::mat4> bones;
};

struct RenderTargetInfo 
{
	//Rect viewport_rect;
	//CameraParams camParams;
};

class IRenderer {
public:
	virtual void Initialize(int width, int height) = 0;

	virtual void SetRenderTarget(UID id, RenderTargetInfo info) = 0;

	virtual void UnsetRenderTarget(UID id) = 0;
	
	virtual bool RenderFrame(const std::vector<RenderableObject>& renderable_objects) = 0;

	virtual void Cleanup() = 0;

	virtual void RegisterRenderingPipeline(std::shared_ptr<RenderingPipeline> rendering_pipeline) = 0;

	virtual void RegisterMaterial(std::shared_ptr<Material> material) = 0;

	virtual void RegisterMesh(std::shared_ptr<Mesh> mesh) = 0;
};