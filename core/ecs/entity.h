#pragma once

namespace ecs {
	typedef std::uint32_t EntityVersion;
	typedef std::uint32_t EntityIndex;

	struct EntityID {
		EntityVersion version;
		EntityIndex index;
	};

	extern const EntityID null_entity_id = { 0, 0 };
}
