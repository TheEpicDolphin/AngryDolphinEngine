#include "physics_system.h"

void PhysicsSystem::Update(float fixedDeltaTime)
{
	std::function<void(EntityID, Rigidbody&)> block =
		[fixedDeltaTime](EntityID entity_id, Rigidbody& rb) {
		rb.velocity += gravity * fixedDeltaTime;
		rb.previous_position = rb.position;
		rb.position += rb.velocity * fixedDeltaTime;
	};
	ecs_->EnumerateComponentsWithBlock<Rigidbody>(block);
}