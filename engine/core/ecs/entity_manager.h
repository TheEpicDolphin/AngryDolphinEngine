#pragma once

#include <iostream>
#include <queue>
#include <bitset>
#include "entity.h"

//Make this singleton
class EntityManager {

public:
	EntityManager();

	~EntityManager();

	Entity CreateEntity();

	void Destroy(EntityId entity_id);

private:
	std::queue<EntityId> unused_ids_;
	uint32_t entity_count_;
};