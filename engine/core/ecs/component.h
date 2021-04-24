#pragma once

#include <core/utils/uid_generator.h>

typedef UID ComponentTypeID;

template <typename T>
struct Component {

public:

	Component(ComponentTypeID type_id) {
		if (!type_id_) {
			type_id_ = type_id;
		}
	}

	static ComponentTypeID GetTypeId() {
		return type_id_;
	}

private:
	static ComponentTypeID type_id_;
};