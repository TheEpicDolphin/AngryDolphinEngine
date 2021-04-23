#pragma once

#include <core/utils/uid_generator.h>

typedef UID InstanceID;

template <typename T>
class Object
{
public:
	Object()
	{
		id_ = uid_generator.CheckoutNewId();
	}

	~Object()
	{
		uid_generator.ReturnId(id_);
		id_ = 0;
	}

	InstanceID GetInstanceID() {
		return id_;
	}

private:
	InstanceID id_;

	static UIDGenerator uid_generator;
};
