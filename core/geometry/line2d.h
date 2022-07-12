#pragma once

#include <glm/vec2.hpp>

namespace geometry {
	struct Line2D {
		glm::vec2 start;
		glm::vec2 end;

		Line2D() = default;
		Line2D(glm::vec2 start, glm::vec2 end);

		bool IsPointOnLeftSide(glm::vec2 p);
	};
}