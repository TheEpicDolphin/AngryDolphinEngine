#pragma once

#include <assert.h>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <functional>

#include <core/utils/set_trie.h>
#include <core/utils/type_info.h>

#include <config/core_components.h>

#include "entity.h"
#include "archetype.h"

enum SystemUpdateType {
	SystemUpdateTypeFixedTimestep = (1u << 0),
	SystemUpdateTypeOnFrame = (1u << 1),
};

namespace ecs {
	class Registry {
	
	public:
		Registry() 
		{
			// This maintains the type ids of components consistent across different compilations.
			#define REGISTER_COMPONENT(name) component_type_info_.GetTypeId<name>();
				CORE_COMPONENTS
			#undef REGISTER_COMPONENT
		}

		template<typename T>
		void AddComponent(EntityID entity_id, T component)
		{
			assert(entity_id.index < entity_archetype_map_.size());
			ComponentTypeID added_component_type = component_type_info_.GetTypeId<T>();
			if (entity_archetype_map_[entity_id.index] != nullptr) {
				// The entity currently belongs to an archetype. This archetype will be
				// referred to as the "previous_archetype"
				Archetype *previous_archetype = entity_archetype_map_[entity_id.index];
				if (std::find(previous_archetype->ComponentTypes().begin(),
					previous_archetype->ComponentTypes().end(),
					added_component_type) != previous_archetype->ComponentTypes().end())
				{
					throw std::runtime_error("Cannot have multiple components of same type on entity");
					return;
				}

				// Determine the entity's new archetype id.
				const std::vector<ComponentTypeID>& previous_component_types = previous_archetype->ComponentTypes();
				std::vector<ComponentTypeID> new_component_types;
				for (std::size_t c_idx = 0; c_idx < previous_component_types.size(); ++c_idx) {
					if (added_component_type < previous_component_types[c_idx]) {
						new_component_types.push_back(added_component_type);
					}
					new_component_types.push_back(previous_component_types[c_idx]);
				}
				if (new_component_types.size() == previous_component_types.size()) {
					new_component_types.push_back(added_component_type);
				}

				Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet(new_component_types);
				if (!existing_archetype) {
					// No archetype exists for the entity's new set of component types. Create
					// new archetype.
					existing_archetype = previous_archetype->EmptyWithAddedComponentType<T>(&component_type_info_);
					archetype_set_trie_.InsertValueForKeySet(*existing_archetype, existing_archetype->ComponentTypes());
				}

				// Move over the entity's component data from the previous archetype to
				// the existing one and insert new component data.
				previous_archetype->MoveEntityToSuperArchetype<T>(entity_id, *existing_archetype, component);
				if (previous_archetype->Entities().size() == 0) {
					// previous archetype no longer has any entities. Delete it.
					DestroyArchetype(previous_archetype);
				}
				entity_archetype_map_[entity_id.index] = existing_archetype;
			}
			else {
				// This entity will be added to an archetype for the first time. This also
				// means that the component to be added will be this entity's first component.
				Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet({ added_component_type });
				if (existing_archetype) {
					// Add entity to existing archetype
					existing_archetype->AddEntity<T>(entity_id, component);
					entity_archetype_map_[entity_id.index] = existing_archetype;
				}
				else {
					// Create new archetype for entity.
					Archetype* new_archetype = Archetype::ArchetypeWithComponentTypes<T>(&component_type_info_);
					new_archetype->AddEntity<T>(entity_id, component);
					archetype_set_trie_.InsertValueForKeySet(*new_archetype, new_archetype->ComponentTypes());
					entity_archetype_map_[entity_id.index] = new_archetype;
				}
			}
		}

		template<typename T>
		void RemoveComponent(EntityID entity_id)
		{
			ComponentTypeID removed_component_type = component_type_info_.GetTypeId<T>();
			assert(entity_id.index < entity_archetype_map_.size());
			if (entity_archetype_map_[entity_id.index] != nullptr) {
				// The entity currently belongs to an archetype. This archetype will be
				// referred to as the "previous_archetype"
				Archetype* previous_archetype = entity_archetype_map_[entity_id.index];
				if (std::find(previous_archetype->ComponentTypes().begin(),
					previous_archetype->ComponentTypes().end(),
					removed_component_type) == previous_archetype->ComponentTypes().end())
				{
					throw std::runtime_error("Attempting to remove component that cannot be found on entity.");
					return;
				}

				// Determine the entity's new archetype id.
				const std::vector<ComponentTypeID> previous_component_types = previous_archetype->ComponentTypes();
				std::vector<ComponentTypeID> new_component_types;
				for (std::size_t c_idx = 0; c_idx < previous_component_types.size(); ++c_idx) {
					if (removed_component_type != previous_component_types[c_idx]) {
						new_component_types.push_back(previous_component_types[c_idx]);
					}
				}

				if (new_component_types.size() == 0) {
					// The entity will now have no components. It is no longer assigned to an archetype.
					previous_archetype->RemoveEntity(entity_id);
					if (previous_archetype->Entities().size() == 0) {
						DestroyArchetype(previous_archetype);

						entity_archetype_map_[entity_id.index] = nullptr;
						return;
					}

					Archetype* existing_archetype = archetype_set_trie_.ValueForKeySet(new_component_types);
					if (!existing_archetype) {
						// No archetype exists for the entity's new set of component types. Create
						// new archetype.
						existing_archetype = previous_archetype->EmptyWithRemovedComponentType<T>(&component_type_info_);
						archetype_set_trie_.InsertValueForKeySet(*existing_archetype, existing_archetype->ComponentTypes());
					}

					// Move over the entity's component data from the previous archetype to
					// the existing one, except that of the component to be removed.
					previous_archetype->MoveEntityToSubArchetype<T>(entity_id, *existing_archetype);
					if (previous_archetype->Entities().size() == 0) {
						DestroyArchetype(previous_archetype);
					}
					entity_archetype_map_[entity_id.index] = existing_archetype;
				}
				else {
					throw std::runtime_error("Attempting to remove component from entity that does not belong to an archetype.");
					return;
				}
			}
		}

		template<typename T>
		bool GetComponent(EntityID entity_id, T& component)
		{
			Archetype *archetype = entity_archetype_map_[entity_id.index];
			if (!archetype) {
				return false;
			}
			return archetype->GetComponentForEntity<T>(entity_id, component);
		}

		template<class... Ts>
		bool GetComponentSet(EntityID entity_id, Ts&... components)
		{
			Archetype* archetype = entity_archetype_map_[entity_id.index];
			if (!archetype) {
				return false;
			}
			return archetype->GetComponentSetForEntity(entity_id, components...);
		}

		template<class... Ts>
		void EnumerateComponentsWithBlock(std::function<void(EntityID entity_id, Ts&...)> block)
		{
			std::vector<Archetype*> archetypes = GetArchetypesWithComponents<Ts...>();
			for (Archetype *archetype : archetypes) {
				archetype->EnumerateComponentsWithBlock<Ts...>(block);
			}
		}

		void RegisterEntity(EntityID entity_id)
		{
			const std::size_t n = entity_id.index + 1 - entity_archetype_map_.size();
			if (n > 0) {
				entity_archetype_map_.insert(entity_archetype_map_.end(), n, nullptr);
			}
		}

		void UnregisterEntity(EntityID entity_id)
		{
			assert(entity_id.index < entity_archetype_map_.size());
			if (entity_archetype_map_[entity_id.index] != nullptr) {
				Archetype* archetype = entity_archetype_map_[entity_id.index];
				archetype->RemoveEntity(entity_id);
				if (archetype->Entities().size() == 0) {
					DestroyArchetype(archetype);
				}
				entity_archetype_map_[entity_id.index] = nullptr;
			}
		}

	private:
		SetTrie<ComponentTypeID, Archetype> archetype_set_trie_;

		std::vector<Archetype*> entity_archetype_map_;

		//std::vector<Archetype> restored_archetypes_;
		TypeInfo component_type_info_;

		// TODO: Consider caching archetypes in systems. When a change to the ECS'
		// Archetype Set Trie is detected (perhaps through some subscriber pattern),
		// Then the system can fetch the desired archetypes again. 
		// std::vector<Archetype *> cached_archetypes_;

		template<class... Ts>
		std::vector<Archetype*> GetArchetypesWithComponents() 
		{
			std::vector<ComponentTypeID> component_types = { (component_type_info_.GetTypeId<Ts>())... };
			return archetype_set_trie_.FindSuperKeySetValues(component_types);
		}

		void DestroyArchetype(Archetype* archetype) 
		{
			archetype_set_trie_.RemoveValueForKeySet(archetype->ComponentTypes());
			delete archetype;
		}
	};
}