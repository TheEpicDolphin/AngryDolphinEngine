
#include "bounds.h"

using namespace geometry;

Bounds::Bounds(glm::vec3 center, glm::vec3 extents) {
	this->min = glm::vec3(center.x - extents.x, center.y - extents.y, center.z - extents.z);
	this->max = glm::vec3(center.x + extents.x, center.y + extents.y, center.z + extents.z);
}

Bounds::Bounds(glm::vec3 min, glm::vec3 max) {
	this->min = min;
	this->max = max;
}

bool Bounds::ContainsPoint(glm::vec3 p) {
	return (min.x <= p.x && p.x <= max.x) &&
		(min.y <= p.y && p.y <= max.y) &&
		(min.z <= p.z && p.z <= max.z);
}