#pragma once

#include <glm/vec3.hpp>

struct Rigidbody 
{
	glm::vec3 position;
	glm::vec3 previous_position;
	glm::vec3 velocity;
	bool interpolate;
};
