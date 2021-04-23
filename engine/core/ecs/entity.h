#pragma once

#include <core/utils/uid_generator.h>

typedef UID EntityID;

class Entity {
	
public:

	Entity(EntityID id) 
	{
		id_ = id;
	}

	~Entity() 
	{
		id_ = 0;
	}

	void destroy();

	EntityID id();

private:
	EntityID id_ = 0;
};