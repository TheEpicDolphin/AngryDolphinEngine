#include "transform.h"

glm::vec3 transform::Right(const glm::mat4& transform) {
	return glm::vec3(transform[0]);
}

glm::vec3 transform::Up(const glm::mat4& transform) {
	return glm::vec3(transform[1]);
}

glm::vec3 transform::Forward(const glm::mat4& transform) {
	return -glm::vec3(transform[2]);
}

glm::vec3 transform::Position(const glm::mat4& transform) {
	return glm::vec3(transform[3]);
}

void transform::SetPosition(glm::mat4& transform, glm::vec3 position) {
	transform[3][0] = position.x;
	transform[3][1] = position.y;
	transform[3][2] = position.z;
}

// Transforms matrix to transform's local space.
glm::mat4 transform::InverseTransformedMatrix(const glm::mat4& transform, const glm::mat4& matrix)
{
	return glm::inverse(transform) * matrix;
}

// Transforms local_matrix from transform's local space.
glm::mat4 transform::TransformedMatrix(const glm::mat4& transform, const glm::mat4& local_matrix)
{
	return transform * local_matrix;
}

// Transforms point to transform's local space.
glm::vec3 transform::InverseTransformedPoint(const glm::mat4& transform, const glm::vec3& point)
{
	return glm::inverse(transform) * glm::vec4(point, 1);
}

// Transforms point from transform's local space.
glm::vec3 transform::TransformedPoint(const glm::mat4& transform, const glm::vec3& local_point)
{
	return transform * glm::vec4(local_point, 1);
}