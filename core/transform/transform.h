#pragma once

#include <glm/mat4x4.hpp>
#include <vector>
#include <core/utils/smart_array.h>

typedef std::uint64_t TransformID;

struct Transform
{
	// ID of self.
	TransformID id;
	// ID of parent. If parent is world, this is 0.
	TransformID parent_id;
	// Children of this transform
	SmartArray<Transform> children;
	// Relative to parent. If this transform is the root, then local_matrix == world_matrix.
	glm::mat4 local_matrix;
	// Relative to world
	glm::mat4 world_matrix;
};