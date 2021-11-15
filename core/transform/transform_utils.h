#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "transform.h"

namespace transform_utils {
	
	glm::vec3 Position(Transform& trans) {
		return glm::vec3(trans.world_matrix[3]);
	}

	glm::mat4 TransformWorldToLocal(glm::mat4 world_matrix, Transform transform) 
	{

	}

	glm::mat4 TransformLocalToWorld(glm::mat4 local_matrix, Transform transform)
	{

	}

}