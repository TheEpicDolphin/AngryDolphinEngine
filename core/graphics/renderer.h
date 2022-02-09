#pragma once

#include "mesh.h"
#include "material.h"
#include "rendering_pipeline.h"

#include <glm/mat4x4.hpp>
#include <core/geometry/rect.h>
#include <core/geometry/bounds.h>

struct RenderableObject
{
	Mesh* mesh;
	Material* material;
	glm::mat4 model_matrix;
	geometry::Bounds aabb;
	std::vector<glm::mat4> bones;
};

/*
struct CameraParams {
	bool is_orthographic;
	// In degrees.
	float vertical_fov;
	float aspect_ratio;
	float near_clip_plane_z;
	float far_clip_plane_z;
	glm::mat4 world_transform;
	Rect viewport_rect;
};
*/

struct CameraParams {
	glm::mat4 view_projection_matrix;
	geometry::Rect viewport_rect;
};

class IRenderer {
public:
	virtual void Initialize(int width, int height) = 0;

	virtual void PreloadRenderingPipeline(const std::shared_ptr<RenderingPipeline>& pipeline) = 0;
	
	virtual bool RenderFrame(const CameraParams& camera_params, const std::vector<RenderableObject>& renderable_objects) = 0;

	virtual void Cleanup() = 0;
};