#pragma once

namespace ecs {
	extern const EntityID null_entity_id = { 0, 0 };

	struct EntityID {
		std::uint32_t version;
		std::uint32_t index;
	};
}
