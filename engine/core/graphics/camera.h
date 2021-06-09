#pragma once

#include <core/utils/Rect.h>

struct Camera {
	bool enabled;
	Rect viewport_rect;
	bool is_orthographic;
};