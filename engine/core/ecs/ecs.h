#pragma once

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <functional>

#include <core/utils/set_trie.h>

#include "entity.h"
#include "component.h"
#include "archetype.h"

class ECS 
{

public:
	template<typename T, typename... Args>
	void AddComponent(EntityID entity_id, Args&&...args)
	{
		static_assert(std::is_base_of<Component<T>, T>::value,
			"Component to add must derive from base class 'Component<T>'" > );

		ComponentTypeID added_component_type = Component<T>::GetTypeId();		 
		if (!added_component_type) {
			// We are adding a component of this type for the first time. Register it.
			
			// ALSO, if we are registering this component, then it is not possible for 
			// this entity to already have this component type.
			added_component_type = RegisterComponent<T>();
		}

		if (entity_archetype_map_.find(entity_id) != entity_archetype_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Archetype *previous_archetype = entity_archetype_map_[entity_id];
			if (std::find(previous_archetype->ComponentTypes().begin(),
				previous_archetype->ComponentTypes().end(),
				added_component_type) != previous_archetype->ComponentTypes().end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return;
			}

			// Determine the entity's new archetype id.
			const std::vector<ComponentTypeID> previous_component_types = previous_archetype->ComponentTypes();
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
				Archetype new_empty_archetype = previous_archetype->EmptyWithAddedComponentType<T>();
				existing_archetype = archetype_set_trie_.InsertValueForKeySet(new_empty_archetype, new_empty_archetype.ComponentTypes());
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			previous_archetype->MoveEntityToSuperArchetype<T>(entity_id, *existing_archetype, T(args, added_component_type));
			if (previous_archetype->Entities().size() == 0) {
				// previous archetype no longer has any entities. Delete it.
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->ComponentTypes());
			}
			entity_archetype_map_[entity_id] = existing_archetype;
		}
		else {
			// This entity will be added to an archetype for the first time. This also
			// means that the component to be added will be this entity's first component.
			Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet({ added_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				existing_archetype->AddEntity<T>(entity_id, T(args, added_component_type));
				entity_archetype_map.insert(std::make_pair(entity_id, existing_archetype));
			}
			else {
				// Create new archetype for entity.
				Archetype unit_archetype = Archetype::UnitArchetypeWithEntity<T>(T(args, added_component_type), entity_id);
				Archetype *const new_archetype = archetype_set_trie_.InsertValueForKeySet(unit_archetype, unit_archetype.ComponentTypes());
				entity_archetype_map_.insert(std::make_pair(entity_id, new_archetype));
			}
		}

		component_count_map_[added_component_type] += 1;
	}

	template<typename T>
	void RemoveComponent(EntityID entity_id)
	{
		ComponentTypeID removed_component_type = Component<T>::GetTypeId();
		if (!removed_component_type) {
			throw std::runtime_error("Cannot remove component that has not been registered");
			return;
		}

		if (entity_archetype_map_.find(entity_id) != entity_archetype_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Archetype *previous_archetype = entity_archetype_map_[entity_id];
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
				previous_archetype->RemoveEntity(entity_id);
				if (previous_archetype->Entities().size() == 0) {
					archetype_set_trie_.RemoveValueForKeySet(previous_archetype->ComponentTypes());
				}
				entity_archetype_map_.erase(entity_id);
				return;
			}
			
			Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet(new_component_types);
			if (!existing_archetype) {
				// No archetype exists for the entity's new set of component types. Create
				// new archetype.
				Archetype new_empty_archetype = existing_archetype->EmptyWithRemovedComponentType<T>();
				existing_archetype = archetype_set_trie_.InsertValueForKeySet(new_empty_archetype, new_empty_archetype.ComponentTypes());
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one, except that of the component to be removed.
			previous_archetype->MoveEntityToSubArchetype<T>(entity_id, *existing_archetype);
			if (previous_archetype->Entities().size() == 0) {
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->ComponentTypes());
			}
			entity_archetype_map_[entity_id] = existing_archetype;
		}
		else {
			throw std::runtime_error("Attempting to remove component from entity that does not belong to an archetype.");
			return;
		}
		
		component_count_map_[removed_component_type] -= 1;
		if (component_count_map_[removed_component_type] == 0) {
			// No entity owns a component of this type anymore. Unregister it.
			UnregisterComponent<T>();
		}
	}

	template<typename T>
	T* GetComponent(EntityID entity_id)
	{
		Archetype *archetype = entity_archetype_map_[entity_id];
		return archetype->GetComponentForEntity<T>(entity_id);
	}

	template<typename T>
	void GetComponentWithSafeBlock(EntityID entity_id, std::function<void(const T&)> success_block, std::function<void()> failure_block)
	{
		Archetype* archetype = entity_archetype_map_[entity_id];
		archetype->GetComponentForEntityWithSafeBlock<T>(entity_id, success_block, failure_block);
	}

	template<class... Ts>
	void EnumerateComponentsWithBlock(std::function<void(EntityID entity_id, Ts&...)> block)
	{
		std::vector<Archetype *> archetypes = GetArchetypesWithComponents<Ts...>();
		for (Archetype *archetype : archetypes) {
			archetype->EnumerateComponentsWithBlock<Ts>(block);
		}
	}

	EntityID CreateEntity() 
	{
		return entity_uid_generator_.CheckoutNewId();
	}

	void DestroyEntity(EntityID entity_id) 
	{
		std::unordered_map<EntityID, Archetype*>::iterator iter = entity_archetype_map_.find(entity_id);
		if (iter != entity_archetype_map_.end()) {
			Archetype* archetype = iter->second;
			archetype->RemoveEntity(entity_id);
			if (archetype->Entities().size() == 0) {
				archetype_set_trie_.RemoveValueForKeySet(archetype->ComponentTypes());
			}
			entity_archetype_map_.erase(entity_id);
		}
		entity_uid_generator_.ReturnId(entity_id);
	}

private:
	SetTrie<ComponentTypeID, Archetype> archetype_set_trie_;
	std::unordered_map<EntityID, Archetype *> entity_archetype_map_;
	std::unordered_map<ComponentTypeID, uint64_t> component_count_map_;
	UIDGenerator component_uid_generator_;
	UIDGenerator entity_uid_generator_;

	template<class... Ts>
	std::vector<Archetype *> GetArchetypesWithComponents() 
	{
		std::vector<ComponentTypeID> component_types = { (Component<Ts>::GetTypeId())... };
		return archetype_set_trie_.FindSuperKeySetValues(component_types);
	}

	template<typename T>
	ComponentTypeID RegisterComponent()
	{
		ComponentTypeID claimed_type_id = component_uid_generator_.CheckoutNewId();
		component_count_map_.insert({ claimed_type_id, 0 });
		return claimed_type_id;
	}

	template<typename T>
	void UnregisterComponent() 
	{
		ComponentTypeId component_type_id = Component<T>::GetTypeId();
		component_count_map_.erase(component_type_id);
		component_uid_generator_.ReturnId(component_type_id);
	}

};