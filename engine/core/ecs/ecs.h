#pragma once

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <functional>

#include <core/utils/set_trie.h>

#include "entity.h"
#include "component.h"
#include "archetype.h"

class ECS {
	
public:
	template<typename T, typename... Args>
	void AddComponent(EntityID entity_id, Args&&...args)
	{
		static_assert(std::is_base_of<Component<T>, T>::value,
			"Component to add must derive from base class 'Component<T>'" > );

		ComponentTypeID added_component_type = Component<T>::GetTypeId();
		if (!added_component_type) {
			// We are adding a component of this type for the first time. Register it.
			added_component_type = RegisterComponent<T>();
		}

		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype;
			if (std::find(previous_archetype->component_types.begin(),
				previous_archetype->component_types.end(),
				added_component_type) != previous_archetype->component_types.end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeID> new_component_types;
			bool has_added_new_component = false;
			for (std::size_t c_idx = 0; c_idx < previous_archetype->component_types.size; ++c_idx) {
				ComponentTypeID component_type = previous_archetype->component_types[c_idx];
				if (added_component_type < component_type && !has_added_new_component) {
					new_component_types.push_back(added_component_type);
					has_added_new_component = true;
				}
				new_component_types.push_back(component_type);
			}
			if (!has_added_new_component) {
				new_component_types.push_back(component_type);
			}

			Archetype *existing_archetype = GetMatchingArchetype(new_component_types);
			if (!existing_archetype) {
				// No archetype exists for the entity's new set of component types. Create
				// new archetype.
				existing_archetype = new Archetype();
				existing_archetype->component_types = new_component_types;
				for (std::size_t c_idx = 0; c_idx < new_component_types.size; ++c_idx) {
					if (new_component_types[c_idx] == added_component_type) {
						existing_archetype->component_arrays.push_back(new ComponentArray<T>());
					}
					else if (new_component_types[c_idx] > added_component_type) {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx - 1]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
					else {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
				}
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			for (std::size_t c_idx = 0; c_idx < existing_archetype->component_types.size; ++c_idx) {
				if (existing_archetype->component_types[c_idx] == added_component_type) {
					existing_archetype->component_arrays[c_idx]->Append(T(args));
				}
				else if (existing_archetype->component_types[c_idx] > added_component_type) {
					ComponentBase comp = previous_archetype->component_arrays[c_idx - 1]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx]->Append(comp);
				}
				else {
					ComponentBase comp = previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx]->Append(comp);
				}
			}

			previous_archetype->entity_ids.pop_back();
			if (previous_archetype->entity_ids.size == 0) {
				// previous archetype no longer has any entities. Delete it.
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->component_types);
				delete previous_archetype;
			}
			else {
				EntityID moved_entity = previous_archetype->entity_ids[record.index];
				entity_archetype_record_map_[moved_entity] = { previous_archetype, record.index };
			}

			std::size_t entity_count = existing_archetype->entity_ids.size;
			Record new_record =
			{
				existing_archetype,
				entity_count
			};
			existing_archetype->entity_ids.push_back(entity_id);
			entity_archetype_record_map_[entity_id] = new_record;
		}
		else {
			// This entity will be added to an archetype for the first time. This also
			// means that the component to be added will be this entity's first component.
			Archetype *existing_archetype = GetMatchingArchetype({ added_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				existing_archetype->component_arrays[0]->Append(T(args));
				
				std::size_t entity_count = existing_archetype->entity_ids.size;
				Record new_record =
				{
					existing_archetype,
					entity_count
				};
				existing_archetype->entity_ids.push_back(entity_id);
				entity_archetype_record_map_.insert({ entity_id, new_record });
			}
			else {
				// Create new archetype for entity.
				Archetype *new_archetype = new Archetype();
				new_archetype->component_types = { added_component_type };
				ComponentArray<T> new_component_array = new ComponentArray<T>();
				new_component_array->Append(T(args));
				new_archetype->component_arrays = { new_component_array };
				new_archetype->entity_ids = { entity_id };
				archetype_set_trie_.InsertValueForKeySet(new_archetype, new_archetype->component_types);
				Record newRecord =
				{
					newArchetype,
					0
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
			}
		}

		Component<T>::count += 1;
	}

	template<typename T>
	void RemoveComponent(EntityID entity_id)
	{
		ComponentTypeID removed_component_type = Component<T>::GetTypeId();
		if (!removed_component_type) {
			throw std::runtime_error("Cannot remove component that has not been registered");
			return;
		}

		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype;
			if (std::find(previous_archetype->component_types.begin(),
				previous_archetype->component_types.end(),
				removed_component_type) == previous_archetype->component_types.end())
			{
				throw std::runtime_error("Attempting to remove component that cannot be found on entity.");
				return;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeID> new_component_types;
			for (std::size_t c_idx = 0; c_idx < previous_archetype->component_types.size; ++c_idx) {
				ComponentTypeID component_type = previous_archetype->component_types[c_idx];
				if (component_type != removed_component_type) {
					new_component_types.push_back(component_type);
				}
			}

			Archetype *existing_archetype = GetMatchingArchetype(new_component_types);
			if (!existing_archetype) {
				// No archetype exists for the entity's new set of component types. Create
				// new archetype.
				existing_archetype = new Archetype();
				existing_archetype->component_types = new_component_types;
				for (std::size_t c_idx = 0; c_idx < previous_archetype->component_types.size; ++c_idx) {
					if (previous_archetype->component_types[c_idx] != removed_component_type) {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
				}
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one, except that of the component to be removed.
			for (std::size_t c_idx = 0; c_idx < previous_archetype->component_types.size; ++c_idx) {
				if (previous_archetype->component_types[c_idx] == removed_component_type) {
					existing_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
				}
				else if (previous_archetype->component_types[c_idx] > removed_component_type) {
					ComponentBase comp = previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx - 1]->Append(comp);
				}
				else {
					ComponentBase comp = previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx]->Append(comp);
				}
			}

			previous_archetype->entityIds.pop_back();
			if (previous_archetype->entityIds.size == 0) {
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->component_types);
			}
			else {
				EntityID moved_entity = previous_archetype->entityIds[record.index];
				entity_archetype_record_map_[moved_entity] = { previous_archetype, record.index };
			}

			std::size_t entity_count = existing_archetype->entityIds.size;
			Record new_record =
			{
				existing_archetype,
				entity_count
			};
			existing_archetype->entityIds.push_back(entity_id);
			entity_archetype_record_map_[entity_id] = new_record;
		}
		else {
			throw std::runtime_error("Attempting to remove component from entity that does not belong to an archetype.");
			return;
		}
		
		if (Component<T>::count > 0) {
			Component<T>::count -= 1;
			if (Component<T>::count == 0) 
			{
				// No entity owns a component of this type anymore. Unregister it.
				UnregisterComponent<T>();
			}
		}
	}

	template<typename T>
	bool GetComponent(EntityID entity_id, std::function<void(const T&)> read_block)
	{
		Record record = entity_archetype_record_map_[entity_id];
		std::size_t component_count = record.archtype->component_types.size;
		ComponentArray<T> *component_array = record.archtype->GetComponentArray<T>();
		if (component_array) {
			const T& component = Component_array->ComponentAtIndex(record.index);
			read_block(component);
			return true;
		}
		else {
			return false;
		}
	}

	template<class... Ts>
	void EnumerateComponentsWithBlock(std::function<void(EntityID entity_id, Ts&...)> block)
	{
		std::vector<Archetype *> archetypes = GetArchetypesWithComponents<Ts>();
		for (Archetype *archetype : archetypes) {
			[block](std::vector<EntityID>& entity_ids, ComponentArray<Ts>* ...component_arrays) {
				std::size_t entity_count = entity_ids.size();
				for (std::size_t e_idx = 0; e_idx < entity_count; ++e_idx) {
					block(entity_ids[e_idx], component_arrays->ComponentAtIndex(e_idx)...);
				}
			}(archetype->entity_ids, archetype->GetComponentArray<Ts>()...);
		}
	}

private:
	struct Record {
		Archetype *archtype;
		std::size_t index;
	};

	SetTrie<ComponentTypeID, Archetype *> archetype_set_trie_;
	std::unordered_map<EntityID, Record> entity_archetype_record_map_;

	Archetype* FindMatchingArchetype(ArchetypeId archtype_Id)
	{
		return archetype_set_trie_.FindValueForKeySet(archtype_Id);
	}

	template<class... Ts>
	std::vector<Archetype *> GetArchetypesWithComponents() 
	{
		std::vector<ComponentTypeID> component_types = { (Component<Ts>::ClaimTypeId())... };
		return archetype_set_trie_.FindSuperSets(component_types);
	}

	template<typename T>
	ComponentTypeID RegisterComponent()
	{
		ComponentTypeID claimed_type_id = Component<T>::ClaimTypeId();
		return claimed_type_id;
	}

	template<typename T>
	void UnregisterComponent() 
	{
		Component<T>::ClearTypeId();
	}

};