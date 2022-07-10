#pragma once

#include <assert.h>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <functional>

#include <core/utils/event_announcer.h>
#include <core/utils/set_trie.h>
#include <core/utils/type_info.h>

#include <config/core_components.h>

#include "entity.h"
#include "archetype.h"

namespace ecs {
	class IComponentSetEventsListener {
	public:
		void OnEntityAcquiredComponentSet(ecs::EntityID entity_id, const ecs::ComponentSetIDs component_set_ids);
		void OnEntityDidLoseComponentSet(ecs::EntityID entity_id, const ecs::ComponentSetIDs component_set_ids);


	};


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
					archetype_set_trie_.InsertValueForKeySet(existing_archetype->ComponentTypes(), *existing_archetype);
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
				Archetype existing_archetype;
				if (archetype_set_trie_.TryGetValueForKeySet({ added_component_type }, existing_archetype)) {
					// Add entity to existing archetype
					existing_archetype.AddEntity<T>(entity_id, component);
					entity_archetype_map_[entity_id.index] = &existing_archetype;
				}
				else {
					// Create new archetype for entity.
					Archetype& new_archetype = archetype_set_trie_.InsertValueForKeySet({ added_component_type }, Archetype());
					Archetype::ConfigureWithComponentSet<T>(new_archetype, &component_type_info_);
					new_archetype.AddEntity<T>(entity_id, component);
					entity_archetype_map_[entity_id.index] = &new_archetype;
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
						archetype_set_trie_.InsertValueForKeySet(existing_archetype->ComponentTypes(), *existing_archetype);
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
			const ComponentSetIDs component_set_ids = GetComponentSetIDs<Ts>();
			
			// Search for an existing component set in component_set_listener_group_trie_.
			// If found, we can simply enumerate the cached archetypes, which is faster
			// than doing a superset search in archetype_set_trie_.
			ComponentSetListenerGroup group;
			std::vector<Archetype*> archetypes;
			if (component_set_listener_group_trie_.TryGetValueForKeySet(component_set_ids, group)) {
				// Faster
				archetypes = group.archetypes;
			} else {
				// Slower
				archetypes = archetype_set_trie_.FindSuperKeySetValues(component_set_ids);
			}

			for (Archetype* archetype : archetypes) {
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

		template<class... Ts>
		ecs::ComponentSetIDs AddComponentSetEventsListener(IComponentSetEventsListener* listener) {
			ComponentSetIDs component_set_ids = GetComponentSetIDs<Ts>();

			ComponentSetListenerGroup group;
			if (component_set_listener_group_trie_.TryGetValueForKeySet(component_set_ids, group)) {
				group.component_set_events_announcer.AddListener(listener);
			} else {
				ComponentSetListenerGroup new_group = component_set_listener_group_trie_.InsertValueForKeySet(component_set_ids, ComponentSetObserverGroup());
				new_group.component_set_ids = component_set_ids;
				new_group.archetypes = archetype_set_trie_.FindSuperKeySetValues(component_set_ids);
				new_group.component_set_events_announcer.AddListener(listener);
			}
		}

		void RemoveComponentSetEventsListener(ecs::ComponentSetIDs component_set_ids, IComponentSetEventsListener* listener) {
			ComponentSetListenerGroup group;
			if (component_set_listener_group_trie_.TryGetValueForKeySet(component_set_ids, group)) {
				group.component_set_events_announcer.RemoveListener(listener);
				if (group.component_set_events_announcer.ListenerCount() == 0) {
					// There are no more remaining listeners for this component set.
					// Delete this component set listener group.
					component_set_listener_group_trie_.RemoveValueForKeySet(component_set_ids);
				}
			}
		}

	private:
		SetTrie<ComponentTypeID, Archetype> archetype_set_trie_;

		std::vector<Archetype*> entity_archetype_map_;

		struct ComponentSetListenerGroup {
			// USE std::includes to compare ComponentSetIDs
			ComponentSetIDs component_set_ids;
			EventAnnouncer<IComponentSetEventsListener> component_set_events_announcer;
			std::vector<Archetype*> archetypes;
		};
		SetTrie<ComponentTypeID, ComponentSetListenerGroup> component_set_listener_group_trie_;

		TypeInfo component_type_info_;

		template<class... Ts>
		const ComponentSetIDs GetComponentSetIDs()
		{
			ComponentSetIDs component_set_ids = { (component_type_info_.GetTypeId<Ts>())... };
			// Sort component set ids from least -> greatest.
			std::sort(component_set_ids.begin(), component_set_ids.end());
			return component_set_ids;
		}

		void DestroyArchetype(Archetype* archetype) 
		{
			archetype_set_trie_.RemoveValueForKeySet(archetype->ComponentSetIDs());
			delete archetype;
		}

		void UpdateComponentSetForEntity(ecs::EntityID entity_id, Archetype& from_archetype, Archetype& to_archetype) {
			std::vector<ComponentSetListenerGroup*> groups = component_set_listener_group_trie_.GetValuesInOrder();
			for (ComponentSetListenerGroup* group : groups) {
				ComponentSetIDs from_archetype_component_set_ids = from_archetype.ComponentSetIDs();
				bool prev = std::includes(
					from_archetype_component_set_ids.begin(),
					from_archetype_component_set_ids.end(),
					group->component_set_ids.begin(),
					group->component_set_ids.end()
				);
				ComponentSetIDs next_archetype_component_set_ids = to_archetype.ComponentSetIDs();
				bool next = std::includes(
					next_archetype_component_set_ids.begin(),
					next_archetype_component_set_ids.end(),
					group->component_set_ids.begin(),
					group->component_set_ids.end()
				);
				if (!prev && next) {
					group->component_set_events_announcer.Announce();
				} else if (prev && !next) {
					group->component_set_events_announcer.Announce();
				}
			}
		}
	};
}