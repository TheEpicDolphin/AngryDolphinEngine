#pragma once

#include <iostream>

#include "type_id_generator.h"

typedef TypeId EntityId;

class Entity {
	
public:

	Entity() 
	{
		id_ = TypeIdGenerator<Entity>().CheckoutNewId();
	}

	~Entity() 
	{
		TypeIdGenerator<Entity>().ReturnId(id_);
		id_ = 0;
	}

	void destroy();

	EntityId id();

private:
	EntityId id_ = 0;
};