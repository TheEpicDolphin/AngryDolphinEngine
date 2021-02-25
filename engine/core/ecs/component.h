#pragma once

#include <iostream>
#include "entity.h"
#include "uid_generator.h"

typedef UID ComponentTypeID;

struct ComponentBase {

};

template <typename T>
struct Component : public ComponentBase {

public:
	static uint64_t count = 0;

	static ComponentTypeID GetTypeId() {
		return type_id_;
	}

	static ComponentTypeID ClaimTypeId() {
		if (type_id_) {
			throw std::runtime_error("Attempting to claim new ComponentTypeID when component already has one.");
			return type_id_;
		}
		type_id_ = TypeIdGenerator<ComponentBase>().CheckoutNewId();
		return type_id_;
	}

	static void ClearTypeId() {
		if (!type_id_) {
			throw std::runtime_error("Attempting to relinquish ComponentTypeID when component doesn't have a valid one yet.");
			return;
		}
		TypeIdGenerator<ComponentBase>().ReturnId(type_id_);
		type_id_ = 0;
	}

private:
	static ComponentTypeID type_id_ = 0;
};