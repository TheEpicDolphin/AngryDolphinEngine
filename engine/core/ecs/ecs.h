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

		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype;
			if (std::find(previous_archetype->ComponentTypes().begin(),
				previous_archetype->ComponentTypes().end(),
				added_component_type) != previous_archetype->ComponentTypes().end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeID> new_component_types;
			bool has_added_new_component = false;
			for (std::size_t c_idx = 0; c_idx < previous_archetype->ComponentTypes().size; ++c_idx) {
				ComponentTypeID component_type = previous_archetype->ComponentTypes()[c_idx];
				if (added_component_type < component_type && !has_added_new_component) {
					new_component_types.push_back(added_component_type);
					has_added_new_component = true;
				}
				new_component_types.push_back(component_type);
			}
			if (!has_added_new_component) {
				new_component_types.push_back(component_type);
			}
			
			Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet(new_component_types);
			if (!existing_archetype) {
				// No archetype exists for the entity's new set of component types. Create
				// new archetype.
				existing_archetype = archetype_set_trie_.InsertValueForKeySet(Archetype(new_component_types), new_component_types);
				for (std::size_t c_idx = 0; c_idx < new_component_types.size; ++c_idx) {
					if (new_component_types[c_idx] == added_component_type) {
						existing_archetype->component_arrays[c_idx] = new ComponentArray<T>();
					}
					else if (new_component_types[c_idx] > added_component_type) {
						existing_archetype->component_arrays[c_idx] = previous_archetype->component_arrays[c_idx - 1]->Empty();
					}
					else {
						existing_archetype->component_arrays[c_idx] = previous_archetype->component_arrays[c_idx]->Empty()
					}
				}
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			for (std::size_t c_idx = 0; c_idx < existing_archetype->ComponentTypes().size; ++c_idx) {
				if (existing_archetype->ComponentTypes()[c_idx] == added_component_type) {
					ComponentArray<T> *casted_existing_component_array = static_cast<ComponentArray<T> *>(existing_archetype->component_arrays[c_idx]);
					casted_existing_component_array->Append(T(args, added_component_type));
				}
				else if (existing_archetype->ComponentTypes()[c_idx] > added_component_type) {
					existing_archetype->component_arrays[c_idx]->AppendComponentFromArrayAtIndex(previous_archetype->component_arrays[c_idx - 1], record.index);
					previous_archetype->component_arrays[c_idx - 1]->RemoveWithSwapAtIndex(record.index);
				}
				else {
					existing_archetype->component_arrays[c_idx]->AppendComponentFromArrayAtIndex(previous_archetype->component_arrays[c_idx], record.index);
					previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
				}
			}

			previous_archetype->entity_ids.pop_back();
			if (previous_archetype->entity_ids.size == 0) {
				// previous archetype no longer has any entities. Delete it.
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->ComponentTypes());
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
			Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet({ added_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				existing_archetype->component_arrays[0]->Append(T(args, added_component_type));
				
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
				Archetype *new_archetype = archetype_set_trie_.InsertValueForKeySet(Archetype({ added_component_type }), { added_component_type });
				ComponentArray<T> *const new_component_array = new ComponentArray<T>();
				new_component_array->Append(T(args, added_component_type));
				new_archetype->component_arrays = { new_component_array };
				new_archetype->entity_ids = { entity_id };
				Record newRecord =
				{
					new_archetype,
					0
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
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

		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			// The entity currently belongs to an archetype. This archetype will be
			// referred to as the "previous_archetype"
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype;
			if (std::find(previous_archetype->ComponentTypes().begin(),
				previous_archetype->ComponentTypes().end(),
				removed_component_type) == previous_archetype->ComponentTypes().end())
			{
				throw std::runtime_error("Attempting to remove component that cannot be found on entity.");
				return;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeID> new_component_types;
			for (std::size_t c_idx = 0; c_idx < previous_archetype->ComponentTypes().size; ++c_idx) {
				ComponentTypeID component_type = previous_archetype->ComponentTypes()[c_idx];
				if (component_type != removed_component_type) {
					new_component_types.push_back(component_type);
				}
			}
			
			Archetype *existing_archetype = archetype_set_trie_.ValueForKeySet(new_component_types);
			if (!existing_archetype) {
				// No archetype exists for the entity's new set of component types. Create
				// new archetype.
				existing_archetype = archetype_set_trie_.InsertValueForKeySet(Archetype(new_component_types), new_component_types);
				for (std::size_t c_idx = 0; c_idx < previous_archetype->ComponentTypes().size; ++c_idx) {
					if (previous_archetype->ComponentTypes()[c_idx] != removed_component_type) {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
				}
			}

			// Move over the entity's component data from the previous archetype to
			// the existing one, except that of the component to be removed.
			for (std::size_t c_idx = 0; c_idx < previous_archetype->ComponentTypes().size; ++c_idx) {
				if (previous_archetype->ComponentTypes()[c_idx] == removed_component_type) {
					existing_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
				}
				else if (previous_archetype->ComponentTypes()[c_idx] > removed_component_type) {
					existing_archetype->component_arrays[c_idx - 1]->AppendComponentFromArrayAtIndex(previous_archetype->component_arrays[c_idx], record.index);
					previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
				}
				else {
					existing_archetype->component_arrays[c_idx]->AppendComponentFromArrayAtIndex(previous_archetype->component_arrays[c_idx], record.index);
					previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
				}
			}

			previous_archetype->entityIds.pop_back();
			if (previous_archetype->entityIds.size == 0) {
				archetype_set_trie_.RemoveValueForKeySet(previous_archetype->ComponentTypes());
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
		
		component_count_map_[removed_component_type] -= 1;
		if (component_count_map_[removed_component_type] == 0) {
			// No entity owns a component of this type anymore. Unregister it.
			UnregisterComponent<T>();
		}
	}

	template<typename T>
	bool GetComponent(EntityID entity_id, std::function<void(const T&)> read_block)
	{
		Record record = entity_archetype_record_map_[entity_id];
		std::size_t component_count = record.archtype->ComponentTypes().size();
		ComponentArray<T> *component_array = record.archtype->GetComponentArray<T>();
		if (component_array) {
			const T& component = component_array[record.index];
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
		std::vector<Archetype *> archetypes = GetArchetypesWithComponents<Ts...>();
		for (Archetype *archetype : archetypes) {
			[block](std::vector<EntityID>& entity_ids, ComponentArray<Ts>* ...component_arrays) {
				std::size_t entity_count = entity_ids.size();
				for (std::size_t e_idx = 0; e_idx < entity_count; ++e_idx) {
					//block(entity_ids[e_idx], component_arrays[e_idx]...);
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

	SetTrie<ComponentTypeID, Archetype> archetype_set_trie_;
	std::unordered_map<EntityID, Record> entity_archetype_record_map_;
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