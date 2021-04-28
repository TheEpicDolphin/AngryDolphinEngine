#pragma once

#include <iostream>
#include "ecs.h"

class SystemBase {

};

template<typename T>
class System : SystemBase
{
public:
	System() = default;

	System(ECS *ecs) {
		ecs_ = ecs;
	}

protected:
	ECS *ecs_;
	// TODO: Consider caching archetypes in systems. When a change to the ECS'
	// Archetype Set Trie is detected (perhaps through some subscriber pattern),
	// Then the system can fetch the desired archetypes again. 
	// std::vector<Archetype *> cached_archetypes_;
};