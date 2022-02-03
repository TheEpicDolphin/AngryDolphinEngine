#pragma once

#include <core/scene/scene.h>
#include <glm/vec3.hpp>

static const glm::vec3 gravity(0.0f, -9.8f, 0.0f);

class RigidbodySystem : public ISystem
{
public:
	RigidbodySystem() = default;

	void OnFixedUpdate(double fixed_delta_time, const IScene& scene) override;

	void OnFrameUpdate(double delta_time, double alpha, const IScene& scene) override;
};