
#include "rigidbody_system.h"

#include <core/ecs/entity.h>
#include <core/scene/scene.h>
#include <core/transform/transform.h>

#include "rigidbody_component.h"

void RigidbodySystem::OnFixedUpdate(double fixed_delta_time, IScene& scene)
{
	std::function<void(ecs::EntityID, RigidbodyComponent&)> block =
		[fixed_delta_time](ecs::EntityID entity_id, RigidbodyComponent& rb) {
		rb.velocity += gravity * (float)fixed_delta_time;
		rb.previous_position = rb.position;
		rb.position += rb.velocity * (float)fixed_delta_time;
	};
	scene.ComponentRegistry().EnumerateComponentsWithBlock<RigidbodyComponent>(block);
}

void RigidbodySystem::OnFrameUpdate(double delta_time, double alpha, IScene& scene)
{
	// Physics interpolation before rendering
	std::function<void(ecs::EntityID, RigidbodyComponent&)> block =
		[alpha, &scene](ecs::EntityID entity_id, RigidbodyComponent& rb) {
		glm::mat4 transform = scene.TransformGraph().GetWorldTransform(entity_id);
		if (rb.interpolate) {
			transform::SetPosition(transform, rb.position * (float)alpha + rb.previous_position * (float)(1.0f - alpha));
		}
		else {
			transform::SetPosition(transform, rb.position);
		}
		scene.TransformGraph().SetWorldTransform(entity_id, transform);
	};
	scene.ComponentRegistry().EnumerateComponentsWithBlock<RigidbodyComponent>(block);
}
