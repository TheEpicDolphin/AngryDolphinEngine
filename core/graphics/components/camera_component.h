#pragma once

#include <core/geometry/rect.h>

struct CameraComponent {
	bool enabled;
	bool is_orthographic;
	float orthographic_half_height;
	// In degrees.
	float vertical_fov;
	float aspect_ratio;
	float near_clip_plane_z;
	float far_clip_plane_z;
	glm::mat4 world_transform;
	geometry::Rect viewport_rect;
};