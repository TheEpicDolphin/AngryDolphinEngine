
#include "rigidbody_system.h"

#include <core/ecs/entity.h>
#include <core/scene/scene.h>
#include <core/utils/transform_utils.h>

#include "rigidbody.h"

void RigidbodySystem::OnFixedUpdate(double fixed_delta_time, const IScene& scene)
{
	std::function<void(EntityID, Rigidbody&)> block =
		[fixedDeltaTime](EntityID entity_id, Rigidbody& rb) {
		rb.velocity += gravity * fixed_delta_time;
		rb.previous_position = rb.position;
		rb.position += rb.velocity * fixed_delta_time;
	};
	scene.Registry().EnumerateComponentsWithBlock<Rigidbody>(block);
}

void RigidbodySystem::OnFrameUpdate(double delta_time, double alpha, const IScene& scene)
{
	// Physics interpolation before rendering
	std::function<void(EntityID, Rigidbody&)> block =
		[alpha, scene](EntityID entity_id, Rigidbody& rb) {
		if (rb.interpolate) {
			scene.TransformGraph().SetWorldTransform(entity_id, rb.position * alpha + rb.previous_position * (1.0f - alpha));
		}
		else {
			scene.TransformGraph().SetWorldTransform(entity_id, rb.position);
		}
	};
	scene.Registry().EnumerateComponentsWithBlock<Rigidbody>(block);
}
