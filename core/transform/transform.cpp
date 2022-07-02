#include "transform.h"

glm::vec3 transform::Left(const glm::mat4& transform) {
	return glm::vec3(transform[0]);
}

glm::vec3 transform::Up(const glm::mat4& transform) {
	return glm::vec3(transform[1]);
}

glm::vec3 transform::Forward(const glm::mat4& transform) {
	return glm::vec3(transform[2]);
}

glm::vec3 transform::Position(const glm::mat4& transform) {
	return glm::vec3(transform[3]);
}

void transform::SetPosition(glm::mat4& transform, glm::vec3 position) {
	transform[3][0] = position.x;
	transform[3][1] = position.y;
	transform[3][2] = position.z;
}

// Transforms world_matrix into transform's local space.
glm::mat4 transform::TransformWorldToLocal(const glm::mat4& transform, const glm::mat4& world_matrix)
{
	return glm::inverse(transform) * world_matrix;
}

// Transforms local_matrix (which is in transform's local space) into world space.
glm::mat4 transform::TransformLocalToWorld(const glm::mat4& transform, const glm::mat4& local_matrix)
{
	return transform * local_matrix;
}

glm::vec3 transform::TransformPointWorldToLocal(const glm::mat4& transform, const glm::vec3& world_point)
{
	return glm::inverse(transform) * glm::vec4(world_point, 1);
}

glm::vec3 transform::TransformPointLocalToWorld(const glm::mat4& transform, const glm::vec3& local_point)
{
	return transform * glm::vec4(local_point, 1);
}