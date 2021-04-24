#pragma once

#include <glm/mat4x4.hpp>

#include <core/ecs/component.h>

struct Transform : public Component<Transform>
{
	glm::mat4 matrix;
};