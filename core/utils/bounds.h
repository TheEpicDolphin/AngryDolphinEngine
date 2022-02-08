#pragma once

#include <glm/vec3.hpp>

namespace geometry {
	struct Bounds {
		glm::vec3 center;
		glm::vec3 extents;
	};
}
