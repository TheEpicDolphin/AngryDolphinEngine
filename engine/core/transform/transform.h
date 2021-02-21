#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include <core/ecs/component.h>

class Transform : public Component<Transform>
{

private:
	glm::mat4 transform_mat_;
};