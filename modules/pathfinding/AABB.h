#pragma once

struct AABB {
	float min[3];
	float max[3];

	bool Intersects(AABB other) const {
		return (min[0] <= other.max[0] && max[0] >= other.min[0]) &&
			(min[1] <= other.max[1] && max[1] >= other.min[1]) &&
			(min[2] <= other.max[2] && max[2] >= other.min[2]);
	}
};