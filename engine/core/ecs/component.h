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
		type_id_ = TypeIdGenerator<ComponentBase>().CheckoutNewId();
		return type_id_;
	}

	static void ClearTypeId() {
		TypeIdGenerator<ComponentBase>().ReturnId(type_id_);
		type_id_ = 0;
	}

private:
	static ComponentTypeId type_id_ = 0;
};