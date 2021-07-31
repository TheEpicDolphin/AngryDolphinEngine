#pragma once

#include "mesh_renderable.h"
#include "rendering_pipeline.h"

#include <glm/mat4x4.hpp>

struct RenderableObjectInfo
{
	std::shared_ptr<Material> material;
	std::shared_ptr<Mesh> mesh;
	glm::mat4 model_matrix;
};

struct RenderTargetInfo 
{
	Rect viewport_rect;
	CameraParams camParams;
};

struct RenderingPipelineInfo 
{

};

struct MaterialInfo 
{

};

class IRenderer {
public:
	virtual void Initialize(int width, int height) = 0;

	virtual void AddRenderableObject(UID id, RenderableObjectInfo info) = 0;

	virtual void RemoveRenderableObject(UID id) = 0;

	virtual void SetRenderTarget(UID id, RenderTargetInfo info) = 0;

	virtual void UnsetRenderTarget(UID id) = 0;
	
	virtual bool RenderFrame() = 0;

	virtual void Cleanup() = 0;

	virtual std::shared_ptr<RenderingPipeline> CreateRenderingPipeline(RenderingPipelineInfo info);

	virtual std::shared_ptr<Material> CreateMaterial(MaterialInfo info);
};