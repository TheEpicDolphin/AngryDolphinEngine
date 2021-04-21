#pragma once

#include <core/utils/typeid_generator.h>

typedef TypeID EntityID;

class Entity {
	
public:

	Entity() 
	{
		id_ = TypeIDGenerator<Entity>().CheckoutNewId();
	}

	~Entity() 
	{
		TypeIDGenerator<Entity>().ReturnId(id_);
		id_ = 0;
	}

	void destroy();

	EntityID id();

private:
	EntityID id_ = 0;
};