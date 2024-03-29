#pragma once

#include <glm/mat4x4.hpp>

namespace ecs {
	struct EntityID;
}

class ITransformService {
public:
	virtual const glm::mat4& GetLocalTransform(ecs::EntityID entity_id) const = 0;

	virtual void SetLocalTransform(ecs::EntityID entity_id, glm::mat4& local_transform_matrix) const = 0;

	virtual const glm::mat4& GetWorldTransform(ecs::EntityID entity_id) const = 0;

	virtual void SetWorldTransform(ecs::EntityID entity_id, glm::mat4& world_transform_matrix) const = 0;

	virtual const ecs::EntityID& GetParent(ecs::EntityID entity_id) const = 0;

	virtual void SetParent(ecs::EntityID entity_id, ecs::EntityID parent_id) const = 0;
};