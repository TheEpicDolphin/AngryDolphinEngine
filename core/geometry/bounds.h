#pragma once

#include <glm/vec3.hpp>

namespace geometry {
	struct Bounds {
		glm::vec3 min;
		glm::vec3 max;

		Bounds(glm::vec3 center, glm::vec3 extents);

		Bounds(glm::vec3 min, glm::vec3 max);

		bool ContainsPoint(glm::vec3 point);
	};
}

