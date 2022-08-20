#pragma once

#include <glm/mat4x4.hpp>
#include <core/geometry/rect.h>
#include <core/geometry/bounds.h>

#include <vector>
#include <memory>

class Mesh;
class Material;
class RenderingPipeline;

struct RenderableObject
{
	Mesh* mesh;
	Material* material;
	glm::mat4 model_matrix;
	geometry::Bounds aabb;
	std::vector<glm::mat4> bones;
};

struct CameraParams {
	glm::mat4 view_projection_matrix;
	geometry::Rect viewport_rect;
};

class IRenderer {
public:
	virtual void PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) = 0;
	virtual void RenderFrame(const CameraParams& camera_params, const std::vector<RenderableObject>& renderable_objects) = 0;
	virtual void Cleanup() = 0;
};