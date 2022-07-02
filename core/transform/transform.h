#pragma once

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

namespace transform {
	glm::vec3 Left(const glm::mat4& transform);
	glm::vec3 Up(const glm::mat4& transform);
	glm::vec3 Forward(const glm::mat4& transform);
	glm::vec3 Position(const glm::mat4& transform);
	void SetPosition(glm::mat4& transform, glm::vec3 position);
	glm::mat4 TransformWorldToLocal(const glm::mat4& transform, const glm::mat4& world_matrix);
	glm::mat4 TransformLocalToWorld(const glm::mat4& transform, const glm::mat4& local_matrix);
	glm::vec3 TransformPointWorldToLocal(const glm::mat4& transform, const glm::vec3& world_point);
	glm::vec3 TransformPointLocalToWorld(const glm::mat4& transform, const glm::vec3& local_point);
}