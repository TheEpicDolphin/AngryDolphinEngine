#pragma once

#include <iostream>
#include <core/ecs/typeid_generator.h>

typedef TypeID InstanceID;

template <typename T>
class Object
{
public:
	Object()
	{
		id_ = TypeIDGenerator<T>().CheckoutNewId();
		return id_;
	}

	~Object()
	{
		TypeIDGenerator<T>().ReturnId(id_);
		id_ = 0;
	}

	InstanceID GetInstanceID() {
		return id_;
	}

private:
	InstanceID id_;
}; #pragma once
