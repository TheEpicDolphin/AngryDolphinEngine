#pragma once

#include <type_traits>
#include <unordered_map>

#include <core/utils/type_id_mapper.h>

class ServiceContainer {
public:
	template<typename B, typename S>
	void BindTo(S& service) {
		static_assert(std::is_base_of<B, S>::value, "Attempting to bind service to a non-base class.");
		service_map_[service_type_id_mapper_.GetTypeId<B>()] = (B*)&service;
	}

	template<typename B>
	bool TryGetService(B*& service_base) {
		auto service_iter = service_map_.find(service_type_id_mapper_.GetTypeId<B>());
		if (service_iter == service_map_.end()) {
			return false;
		}

		service_base = (B*)service_iter->second;
		return true;
	}

	template<typename B>
	void Unbind() {
		service_map_.erase(service_type_id_mapper_.GetTypeId<B>());
	}

private:
	std::unordered_map<std::uint32_t, void*> service_map_;
	TypeIDMapper service_type_id_mapper_;
};