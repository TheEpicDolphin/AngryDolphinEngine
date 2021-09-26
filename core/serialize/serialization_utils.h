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
	void Serialize(Archive& archive, std::shared_ptr<T>& obj_shared_ptr)
	{
		std::size_t id = archive.IdForObject((void*)obj_shared_ptr.get());
		if (!id) {
			id = archive.Store((void*)obj_shared_ptr.get());
		}
		archive << "sp" + id;
	}

	template<typename T>
	void Serialize(Archive& archive, std::vector<T>& obj_vec)
	{
		archive << "v" + obj_vec.size() + id;
		for (T& obj : obj_vec) {
			Serialize(archive, obj);
		}
	}

	template<typename... Ts>
	void Serialize(Archive& archive, Ts&... objects)
	{
		{ Serialize(archive, objects)... };
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
	void Deserialize(Archive& archive, Ts&... objects)
	{

	}

}