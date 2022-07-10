#pragma once

#include <glm/vec3.hpp>

#include <core/definitions/transform/transform_service.h>
#include <core/ecs/system.h>
#include <core/scene/scene.h>
#include <core/ecs/entity.h>
#include <core/transform/transform.h>

#include "rigidbody_component.h"

static const glm::vec3 gravity(0.0f, -9.8f, 0.0f);

class RigidbodySystem : public ISystem
{
public:
	RigidbodySystem() = default;

	void Initialize(ServiceContainer service_container) {
		if (!service_container.TryGetService(component_registry_)) {
			// TODO: Throw error.
		}

		if (!service_container.TryGetService(transform_service_)) {
			// TODO: Throw error.
		}
	}

	void Cleanup(ServiceContainer service_container) {};

	void OnFixedUpdate(double fixed_delta_time)
	{
		std::function<void(ecs::EntityID, RigidbodyComponent&)> block =
			[fixed_delta_time](ecs::EntityID entity_id, RigidbodyComponent& rb) {
			rb.velocity += gravity * (float)fixed_delta_time;
			rb.previous_position = rb.position;
			rb.position += rb.velocity * (float)fixed_delta_time;
		};
		component_registry_->EnumerateComponentsWithBlock<RigidbodyComponent>(block);
	}

	void OnFrameUpdate(double delta_time, double alpha)
	{
		// Physics interpolation before rendering
		std::function<void(ecs::EntityID, RigidbodyComponent&)> block =
			[this, alpha](ecs::EntityID entity_id, RigidbodyComponent& rb) {
			glm::mat4 transform = transform_service_->GetWorldTransform(entity_id);
			if (rb.interpolate) {
				transform::SetPosition(transform, rb.position * (float)alpha + rb.previous_position * (float)(1.0f - alpha));
			}
			else {
				transform::SetPosition(transform, rb.position);
			}
			transform_service_->SetWorldTransform(entity_id, transform);
		};
		component_registry_->EnumerateComponentsWithBlock<RigidbodyComponent>(block);
	}

private:
	ecs::Registry* component_registry_;
	ITransformService* transform_service_;
};