#pragma once

#include <iostream>
#include <cstring>
#include <unordered_map>
#include <functional>

#include "type_id_generator.h"
#include "entity.h"
#include "component.h"
#include "archetype.h"

class ECS {

	template<typename T, typename... Args>
	void AddComponent(EntityId entity_id, Args&&...args)
	{
		static_assert(std::is_base_of<Component<T>, T>::value,
			"Component to add must derive from base class 'Component<T>'" > );

		ComponentTypeId new_component_type = Component<T>::GetTypeId();
		if (!new_component_type) {
			// We are adding a component of this type for the first time. Register it.
			new_component_type = RegisterComponent<T>();
		}

		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype;
			if (std::find(previous_archetype->component_types.begin(),
				previous_archetype->componen_types.end(),
				new_component_type) != previous_archetype->component_types.end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return nullptr;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeId> new_component_types;
			bool has_added_new_component = false;
			for (std::size_t i = 0; i < previous_archetype->component_types.size; ++i) {
				if (new_component_type < previous_archetype->component_types[i] &&
					!has_added_new_component) {
					new_component_types.push_back(new_component_type);
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
					if (new_component_types[c_idx] == new_component_type) {
						existing_archetype->component_arrays.push_back(new ComponentArray<T>());
					}
					else if (new_component_types[c_idx] > new_component_type) {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx - 1]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
					else {
						ComponentArrayBase *empty = previous_archetype->component_arrays[c_idx]->Empty();
						existing_archetype->component_arrays.push_back(empty);
					}
				}
			}

			// Copy over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			for (std::size_t c_idx = 0; c_idx < existing_archetype->component_types.size; ++c_idx) {
				if (existing_archetype->component_types[c_idx] == new_component_type) {
					existing_archetype->component_arrays[c_idx]->Append(T(args));
				}
				else if (existing_archetype->component_types[c_idx] > new_component_type) {
					ComponentBase comp = previous_archetype->component_arrays[c_idx - 1]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx]->Append(comp);
				}
				else {
					ComponentBase comp = previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(record.index);
					existing_archetype->component_arrays[c_idx]->Append(comp);
				}
			}

			previous_archetype->entityIds.pop_back();
			if (previous_archetype->entityIds.size == 0) {
				archetypes_.erase(previous_archetype);
			}
			else {
				EntityId moved_entity = previous_archetype->entityIds[record.index];
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
			// TODO: FIX THIS

			Archetype *existing_archetype = GetMatchingArchetype({ new_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				existing_archetype->component_arrays[0]->Append(T(args));
				
				std::size_t entity_count = existing_archetype->entityIds.size;
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
				new_archetype->component_types = { new_component_type };
				ComponentArray<T> new_component_array = new ComponentArray<T>();
				new_component_array->Append(T(args));
				new_archetype->component_arrays = { new_component_array };
				new_archetype->entity_ids = { entity_id };
				archetypes_.push_back(new_archetype);
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
	void RemoveComponent(Entity entity)
	{
		// TODO: Do stuff
		
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
	void GetComponent(EntityId entity_id, std::function<void(const T&)> read_block)
	{
		Record record = entity_archetype_record_map_[entity_id];
		std::size_t component_count = record.archtype->component_types.size;
		ComponentArray<T> *component_array = record.archtype->GetComponentArray<T>();
		if (component_array) {
			component_array->ReadComponentAtIndex(record.index, read_block);
		}
		else {
			read_block(NULL);
		}
	}

private:
	struct Record {
		Archetype *archtype;
		std::size_t index;
	};

	std::vector<Archetype *> archetypes_;
	std::unordered_map<EntityId, Record> entity_archetype_record_map_;
	std::unordered_map<ComponentTypeId, ComponentBase*> type_component_base_map_;

	Archetype *FindMatchingArchetype(ArchetypeId archtype_Id)
	{
		for (std::vector<Archetype>::iterator it = archetypes_.begin(); it != archetypes_.end(); ++it) {
			if (archtype_Id == it->component_types) {
				return &(*it);
			}
		}
		return nullptr;
	}

	template<typename T>
	ComponentTypeId RegisterComponent() 
	{
		TypeId claimed_type_id = Component<T>::ClaimTypeId();
		type_component_base_map_.insert({ claimed_type_id, new Component<T>() });
		return claimed_type_id;
	}

	template<typename T>
	void UnregisterComponent() 
	{
		type_component_base_map_.erase(Component<T>::GetTypeId());
		Component<T>::ClearTypeId();
	}

};