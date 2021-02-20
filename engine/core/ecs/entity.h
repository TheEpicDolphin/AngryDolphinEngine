#pragma once

#include <iostream>

#include "uid_generator.h"

typedef UID EntityID;

class Entity {
	
public:

	Entity() 
	{
		id_ = UIDGenerator<Entity>().CheckoutNewId();
	}

	~Entity() 
	{
		UIDGenerator<Entity>().ReturnId(id_);
		id_ = 0;
	}

	void destroy();

	EntityID id();

private:
	EntityID id_ = 0;
};