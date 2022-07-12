
#include "line2d.h"

using namespace geometry;

Line2D::Line2D(glm::vec2 start, glm::vec2 end) {
	this->start = start;
	this->end = end;
}

bool Line2D::IsPointOnLeftSide(glm::vec2 p) {
	return ((end.x - start.x) * (p.y - start.y) - (end.y - start.y) * (p.x - start.x)) > 0;
}
