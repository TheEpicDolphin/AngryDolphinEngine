#pragma once

#include <glm/mat4x4.hpp>
#include <core/ecs/entity.h>

struct Transform
{
	// Relative to parent. If this transform is the root, then local_matrix == world_matrix.
	glm::mat4 local_matrix;
	// Relative to world.
	glm::mat4 world_matrix;
};