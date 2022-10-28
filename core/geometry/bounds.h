#pragma once

#include <glm/vec3.hpp>

namespace geometry {
	struct Bounds3f {
		glm::vec3 min;
		glm::vec3 max;

		Bounds3f() = default;
		Bounds3f(glm::vec3 center, float extent_x, float extent_y, float extent_z);
		Bounds3f(glm::vec3 min, glm::vec3 max);

		bool ContainsPoint(glm::vec3 point);
		bool Intersects(Bounds3f other);
	};
}

