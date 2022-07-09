#pragma once

#include <core/geometry/rect.h>

struct CameraComponent {
	bool disabled;
	bool is_orthographic;
	float orthographic_half_height;
	// In degrees.
	float vertical_fov;
	float aspect_ratio;
	float near_clip_plane_z;
	float far_clip_plane_z;
	geometry::Rect viewport_rect;
};