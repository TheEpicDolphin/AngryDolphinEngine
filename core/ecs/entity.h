#pragma once

#include <cinttypes>

namespace ecs {
	typedef std::uint32_t EntityVersion;
	typedef std::uint32_t EntityIndex;

	struct EntityID {
		EntityVersion version;
		EntityIndex index;

		friend bool operator==(const EntityID& lhs, const EntityID& rhs);
		friend bool operator!=(const EntityID& lhs, const EntityID& rhs);
	};

	extern const EntityID null_entity_id;
}
