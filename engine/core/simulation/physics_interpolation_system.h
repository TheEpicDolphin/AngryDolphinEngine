#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>

#include "rigidbody.h"

class PhysicsInterpolationSystem : public System<PhysicsInterpolationSystem>
{
public:
	PhysicsInterpolationSystem(ECS ecs) : System<PhysicsInterpolationSystem>(ecs) {

	}

	void Update(float alpha)
	{
		std::function<void(EntityID, Rigidbody&, Transform&)> block =
		[alpha](EntityID entity_id, Rigidbody& rb, Transform& trans) {
			if (rb.interpolate) {
				trans.position = rb.position * alpha + rb.previous_position * (1.0f - alpha);
			}
			else {
				trans.position = rb.position;
			}
		};
		ecs_.EnumerateComponentsWithBlock<Rigidbody, Transform>(block);
	}
};