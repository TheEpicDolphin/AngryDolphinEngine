#pragma once

namespace ecs {
	struct EntityID;
}

class ISceneEntityInstantiator {
	virtual ecs::EntityID CreateEntity() = 0;

	virtual void DestroyEntity(ecs::EntityID entity_id) = 0;
};