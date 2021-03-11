#pragma once

#include <iostream>
#include "typeid_generator.h"

typedef TypeID InstanceID;

template <typename T>
class Object 
{
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

	public InstanceID GetInstanceID() {
		return id_;
	}

private:
	InstanceID id_;
};