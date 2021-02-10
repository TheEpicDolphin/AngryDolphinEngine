#pragma once

#include <iostream>
#include <unordered_map>
#include "system.h"

class SystemManager {

	template<typename T>
	void RegisterSystem()
	{
		const char* systemTypeName = typeid(T).name();
		BaseSystem system(signature);
		system_map_.insert(systemTypeName, system);
	}

	template<typename T>
	void TrackEntity(Entity entity) 
	{
		const char* systemTypeName = typeid(T).name();
		std::shared_ptr<System> system_ptr = system_map_[systemTypeName];
		system_ptr->TrackEntity(entity);
	}

	template<typename T>
	void UntrackEntity(Entity entity) {
		const char* systemTypeName = typeid(T).name();
		std::shared_ptr<System> system_ptr = system_map_[systemTypeName];
		system_ptr->UntrackEntity(entity);
	}

private:
	std::unordered_map<const char*, std::shared_ptr<System>> system_map_;
};