#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "transform.h"

namespace transform_utils {
	
	glm::vec3 Position(Transform& trans) {
		return glm::vec3(trans.matrix[3]);
	}

	void SetPosition(Transform& trans, glm::vec3& position) {
		trans.matrix[3] = glm::vec4(position, 1.0f);
	}

}