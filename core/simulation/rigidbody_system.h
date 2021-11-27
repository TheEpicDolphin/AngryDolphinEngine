#pragma once

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <glm/vec3.hpp>

static const glm::vec3 gravity(0.0f, -9.8f, 0.0f);

class RigidbodySystem : public ISystem
{
public:
	RigidbodySystem() = default;

	void OnFixedUpdate(double fixed_delta_time, const Scene& scene) override;

	void OnFrameUpdate(double delta_time, double alpha, const Scene& scene) override;
};