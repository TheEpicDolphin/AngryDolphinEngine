#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <glm/vec3.hpp>

#include "rigidbody.h"

static const glm::vec3 gravity(0.0f, -9.8f, 0.0f);

class PhysicsSystem : public System<PhysicsSystem>
{
public:
	PhysicsSystem() = default;

	PhysicsSystem(ECS *ecs) : System<PhysicsSystem>(ecs) {

	}

	void Update(float fixedDeltaTime)
	{
		std::function<void(EntityID, Rigidbody&)> block = 
		[fixedDeltaTime] (EntityID entity_id, Rigidbody& rb) {
			rb.velocity += gravity * fixedDeltaTime;
			rb.previous_position = rb.position;
			rb.position += rb.velocity * fixedDeltaTime;
		};
		ecs_->EnumerateComponentsWithBlock<Rigidbody>(block);
	}
};