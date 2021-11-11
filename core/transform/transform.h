#pragma once

#include <glm/mat4x4.hpp>
#include <core/ecs/entity.h>

struct Transform
{
	// ID of parent. If this is 0, parent is world.
	EntityID parent_id;
	// ID of first child. If this is 0, this transform has no children.
	EntityID first_child_id;
	// Number of children.
	std::size_t child_count;
	// Relative to parent. If this transform is the root, then local_matrix == world_matrix.
	glm::mat4 local_matrix;
	// Relative to world.
	glm::mat4 world_matrix;
};