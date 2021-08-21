#pragma once

#include <glm/mat4x4.hpp>

typedef std::uint64_t TransformID;

struct Transform
{
	// ID of parent. If no parent, this is 0.
	TransformID parent_id;
	// Relative to parent. If this transform is the root, then local_matrix == world_matrix.
	glm::mat4 local_matrix;
	// Relative to world
	glm::mat4 world_matrix;
};