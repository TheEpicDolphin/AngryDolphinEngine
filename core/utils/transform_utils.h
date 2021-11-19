#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace transform_utils {
	
	glm::vec3 Position(glm::mat4 transform_matrix) {
		return glm::vec3(transform_matrix[3]);
	}

	glm::mat4 TransformWorldToLocal(const glm::mat4& world_matrix, const glm::mat4& transform_matrix)
	{
		return world_matrix * glm::inverse(transform_matrix);
	}

	glm::mat4 TransformLocalToWorld(const glm::mat4& local_matrix, const glm::mat4& transform_matrix)
	{
		return local_matrix * transform_matrix;
	}

}