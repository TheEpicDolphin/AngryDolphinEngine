#pragma once

#include <glm/vec3.hpp>

namespace geometry {
	struct Bounds {
		glm::vec3 min;
		glm::vec3 max;

		Bounds() = default;

		Bounds(glm::vec3 center, float extent_x, float extent_y, float extent_z);

		Bounds(glm::vec3 min, glm::vec3 max);

		bool ContainsPoint(glm::vec3 point);
	};
}

