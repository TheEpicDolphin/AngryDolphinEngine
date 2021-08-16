#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>
#include <core/transform/transform_utils.h>

#include "rigidbody.h"

using namespace transform_utils;

class PhysicsInterpolationSystem : public System<PhysicsInterpolationSystem>
{
public:
	PhysicsInterpolationSystem() = default;

	PhysicsInterpolationSystem(ECS *ecs) : System<PhysicsInterpolationSystem>(ecs) {

	}

	void Update(float alpha)
	{
		std::function<void(EntityID, Rigidbody&, Transform&)> block =
		[alpha](EntityID entity_id, Rigidbody& rb, Transform& trans) {
			if (rb.interpolate) {
				SetPosition(trans, rb.position * alpha + rb.previous_position * (1.0f - alpha));
			}
			else {
				SetPosition(trans, rb.position);
			}
		};
		ecs_->EnumerateComponentsWithBlock<Rigidbody, Transform>(block);
	}
};