#pragma once

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>

// In this game engine:
// 
// 1. Forward is along the -z axis
// 2. Up is along the +y axis
// 3. Righ is along the +x axis
//

namespace transform {
	glm::vec3 Forward(const glm::mat4& transform);
	glm::vec3 Up(const glm::mat4& transform);
	glm::vec3 Right(const glm::mat4& transform);
	glm::vec3 Position(const glm::mat4& transform);
	void SetPosition(glm::mat4& transform, glm::vec3 position);
	glm::mat4 InverseTransformedMatrix(const glm::mat4& transform, const glm::mat4& matrix);
	glm::mat4 TransformedMatrix(const glm::mat4& transform, const glm::mat4& local_matrix);
	glm::vec3 InverseTransformedPoint(const glm::mat4& transform, const glm::vec3& point);
	glm::vec3 TransformedPoint(const glm::mat4& transform, const glm::vec3& local_point);
}