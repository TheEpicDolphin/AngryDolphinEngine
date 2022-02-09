
#include "bounds.h"

using namespace geometry;

Bounds::Bounds(glm::vec3 center, float extent_x, float extent_y, float extent_z) {
	this->min = glm::vec3(center.x - extent_x, center.y - extent_y, center.z - extent_z);
	this->max = glm::vec3(center.x + extent_x, center.y + extent_y, center.z + extent_z);
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