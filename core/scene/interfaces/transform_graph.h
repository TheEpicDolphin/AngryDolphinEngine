#pragma once

#include <glm/mat4x4.hpp>
#include <core/ecs/entity.h>

class ITransformGraph {
	virtual const glm::mat4& GetLocalTransform(EntityID id) = 0;

	virtual void SetLocalTransform(EntityID id, glm::mat4& local_transform_matrix) = 0;

	virtual const glm::mat4& GetWorldTransform(EntityID id) = 0;

	virtual void SetWorldTransform(EntityID id, glm::mat4& world_transform_matrix) = 0;

	virtual const EntityID& GetParent(EntityID id) = 0;

	virtual void SetParent(EntityID id, EntityID parent_id) = 0;
};