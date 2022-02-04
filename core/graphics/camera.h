#pragma once

#include <core/utils/Rect.h>

struct CameraComponent {
	bool enabled;
	bool is_orthographic;
	// In degrees.
	float vertical_fov;
	float aspect_ratio;
	float near_clip_plane_z;
	float far_clip_plane_z;
	glm::mat4 world_transform;
	Rect viewport_rect;
};