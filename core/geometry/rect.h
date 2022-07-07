#pragma once

#include <glm/vec2.hpp>

namespace geometry {
	struct Rect {
		glm::vec2 origin;
		glm::vec2 size;

		Rect();
		Rect(float x, float y, float width, float height);
		Rect(glm::vec2 origin, glm::vec2 size);
	};
}
