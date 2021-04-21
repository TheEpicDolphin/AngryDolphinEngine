#pragma once

#include <core/utils/typeid_generator.h>

typedef TypeID InstanceID;

template <typename T>
class Object
{
public:
	Object()
	{
		id_ = TypeIDGenerator<T>().CheckoutNewId();
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
};
