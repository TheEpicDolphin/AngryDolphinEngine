#pragma once

#include <glm/mat4x4.hpp>
#include <vector>

typedef std::uint64_t TransformID;

struct Transform
{
	// ID of self.
	TransformID id;
	// ID of parent. If parent is world, this is 0.
	TransformID parent_id;
	// TODO: Create custom container for storing Transforms contiguously and not invalidating pointers
	// Children of this transform
	std::vector<Transform> children;
	// Relative to parent. If this transform is the root, then local_matrix == world_matrix.
	glm::mat4 local_matrix;
	// Relative to world
	glm::mat4 world_matrix;
};