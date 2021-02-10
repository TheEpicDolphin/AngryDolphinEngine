
#include "entity_manager.h"
#include "entity.h"

// TODO: make 0 and invalid EntityId

EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

Entity EntityManager::CreateEntity()
{
	if (unused_ids_.size() > 0) {
		EntityId id = unused_ids_.front();
		unused_ids_.pop();
		return Entity(id);
	}
	else {
		return Entity(entity_count_++);
	}
}

void EntityManager::Destroy(EntityId entity_id)
{
	entity_count_--;
	unused_ids_.push(entity_id);
}