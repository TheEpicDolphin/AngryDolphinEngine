#include "entity.h"

const ecs::EntityID ecs::null_entity_id = { 0, 0 };

bool ecs::operator==(const ecs::EntityID& lhs, const ecs::EntityID& rhs) {
	return (lhs.version == rhs.version) && (lhs.index == rhs.index);
}

bool ecs::operator!=(const ecs::EntityID& lhs, const ecs::EntityID& rhs) {
	return !(lhs == rhs);
}