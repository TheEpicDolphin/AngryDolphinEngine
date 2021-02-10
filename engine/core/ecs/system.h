#pragma once

#include <iostream>
#include <set>
#include "entity.h"


class System
{
public:
	System() {
		
	}

	void TrackEntity(Entity entity) {
		entities_.insert(entity);
	}

	void UntrackEntity(Entity entity) {
		entities_.erase(entity);
	}

	bool IsTracking(Entity entity) {
		return entities_.find(element) != entities_.end()
	}

private:
	std::set<Entity> entities_;
};