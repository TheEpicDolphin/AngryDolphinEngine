#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "transform.h"

namespace transform_utils {
	
	glm::vec3 Position(Transform& trans) {
		return glm::vec3(trans.world_matrix[3]);
	}

}