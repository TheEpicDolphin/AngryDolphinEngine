#pragma once

#include <core/utils/uid_generator.h>

typedef UID ComponentTypeID;

template <typename T>
class Component {

public:

	static ComponentTypeID GetTypeId() {
		return type_id_;
	}

	static void SetTypeId(ComponentTypeID type_id) {
		type_id_ = type_id;
	}

	static void ClearTypeId() {
		type_id_ = 0;
	}

private:
	static ComponentTypeID type_id_;
};

/* initialized to invalid id of 0 */
template <typename T>
ComponentTypeID Component<T>::type_id_ = 0;