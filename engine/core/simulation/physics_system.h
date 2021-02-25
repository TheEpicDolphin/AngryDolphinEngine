#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>

#include "rigidbody.h"

class PhysicsSystem : public System<PhysicsSystem>
{
public:
	PhysicsSystem(ECS ecs) : System<PhysicsSystem>(ecs) {

	}

	void Update(double fixedDeltaTime)
	{
		std::function<void(EntityID, Rigidbody&)> block = 
		[fixedDeltaTime] (EntityID entity_id, Rigidbody& rb) {
			rb.velocity += gravity_ * fixedDeltaTime;
			rb.previous_position = rb.position;
			rb.position += rb.velocity * fixedDeltaTime;
		};
		ecs_.EnumerateComponentsWithBlock<Rigidbody>(block);
	}

	glm::vec3 gravity_(0.0f, -9.8f, 0.0f);
};