#pragma once

#include <iostream>
#include "entity.h"
#include "type_id_generator.h"

typedef TypeId ComponentTypeId;

class ComponentBase {

};

template <typename T>
class Component : ComponentBase {

public:
	static uint64_t count = 0;

	static ComponentTypeId GetTypeId() {
		return type_id_;
	}

	static ComponentTypeId ClaimTypeId() {
		if (type_id_) {
			throw std::runtime_error("Attempting to claim new ComponentTypeId when component already has one.");
			return type_id_;
		}
		type_id_ = TypeIdGenerator<ComponentBase>().CheckoutNewId();
		return type_id_;
	}

	static void ClearTypeId() {
		if (!type_id_) {
			throw std::runtime_error("Attempting to relinquish ComponentTypeId when component doesn't have a valid one yet.");
			return;
		}
		TypeIdGenerator<ComponentBase>().ReturnId(type_id_);
		type_id_ = 0;
	}

private:
	static ComponentTypeId type_id_ = 0;
};