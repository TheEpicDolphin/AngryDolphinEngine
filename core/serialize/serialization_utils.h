#pragma once

#include "archive.h"

namespace serialization_utils {

	template<typename T>
	void Serialize(Archive& archive, T& object)
	{
		if (std::is_pointer<T>::value) {
			std::size_t id = archive.IdForObject((void*)object);
			if (!id) {
				id = archive.Store((void*)object);
			}
			archive << "p" + id;
		}
		else {
			if (!archive.IdForObject((void*)&object)) {
				archive.Store((void*)&object);
			}
			archive << "o" + id + object.Serialize(archive);
		}
	}

	template<typename T>
	void Deserialize(Archive& archive, T& object)
	{
		if (std::is_pointer<T>::value) {
			std::size_t id = archive.IdForObject((void*)object);
			if (!id) {
				id = archive.Store((void*)object);
			}
			archive << "p" + id;
		}
		else {
			if (!archive.IdForObject((void*)&object)) {
				archive.Store((void*)&object);
				archive.BroadcastObjectToPointers((void*)&object);
			}
			archive << "o" + id + object.Serialize(archive);
		}
	}

	template<typename... Ts>
	void Serialize(Archive& archive, Ts&... objects)
	{

	}

	template<typename... Ts>
	void Deserialize(Archive& archive, Ts... objects)
	{

	}

}