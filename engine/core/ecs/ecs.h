#pragma once

#include <iostream>
#include <cstring>
#include <unordered_map>

#include "type_id_generator.h"
#include "entity.h"
#include "component.h"
#include "archetype.h"

class ECS {

	template<typename T, typename... Args>
	T* AddComponent(EntityId entity_id, Args&&...args)
	{
		static_assert(std::is_base_of<Component<T>, T>::value,
			"Component to add must derive from base class 'Component<T>'" > );

		ComponentTypeId new_component_type = Component<T>::GetTypeId();
		if (!new_component_type) {
			// We are adding a component of this type for the first time. Register it.
			new_component_type = RegisterComponent<T>();
		}

		T *added_component;
		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype_ptr;
			if (std::find(previous_archetype->componentTypes.begin(),
				previous_archetype->componentTypes.end(),
				new_component_type) != previous_archetype->componentTypes.end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return nullptr;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeId> new_component_types;
			bool has_added_new_component = false;
			for (std::size_t i = 0; i < previous_archetype->componentTypes.size; ++i) {
				if (new_component_type < previous_archetype->componentTypes[i] &&
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
				existing_archetype = new Archetype(new_component_types);
			}

			std::size_t entity_count = existing_archetype->entityIds.size;

			// Copy over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			for (std::size_t c_idx = 0; c_idx < existing_archetype->componentTypes.size; ++c_idx) {
				if (existing_archetype->componentTypes[c_idx] == new_component_type) {
					existing_archetype->component_arrays[c_idx]->Append(T(args));
					added_component = &existing_archetype->component_arrays[c_idx]->ItemAtIndex(entity_count);
				}
				else if (existing_archetype->componentTypes[c_idx] > new_component_type) {
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

			Record new_record =
			{
				existing_archetype,
				entity_count
			};
			existing_archetype->entityIds.push_back(entity_id);
			entity_archetype_record_map_[entity_id] = new_record;
		}
		else {
			Archetype *existing_archetype = GetMatchingArchetype({ new_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				std::size_t entity_count = existing_archetype->entityIds.size;
				std::size_t new_data_size = (entity_count + 1) * sizeof(T);
				std::size_t new_alloc_size = new_data_size;
				if (new_data_size > existing_archetype->componentArrayAllocSizes[0]) {
					new_alloc_size *= 2;
				}
				char *componentArray = new char[new_alloc_size];
				ComponentBase *const componentBase = type_component_base_map_[new_component_type];
				for (std::size_t i = 0; i < entity_count; ++i) {
					void *componentSource = &existing_archetype->componentArrays[0][i * sizeof(T)];
					componentBase->CopyData(componentSource, &componentArray[i * sizeof(T)]);
					componentBase->DeleteData(componentSource);
				}
				new_component = new (&componentArray[entity_count * sizeof(T)]) T();
				delete[] existing_archetype->componentArrays[0];
				existing_archetype->componentArrays[0] = componentArray;
				existing_archetype->entityIds.push_back(entity_id);
				existing_archetype->componentArrayAllocSizes[0] = new_alloc_size;
				Record newRecord =
				{
					existing_archetype,
					entity_count
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
			}
			else {
				// Create new archetype for entity.
				Archetype *newArchetype = new Archetype();
				newArchetype->componentTypes = { new_component_type };
				newArchetype->componentArrays = { new char[sizeof(T)] };
				new (&newArchetype->componentArrays[0]) T();
				newArchetype->componentArrayAllocSizes = sizeof(T);
				archetypes_.push_back(newArchetype);
				Record newRecord =
				{
					newArchetype,
					0
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
			}
		}

		Component<T>::count += 1;
		return added_component;
	}

	/*
	template<typename T, typename... Args>
	T* AddComponent(EntityId entity_id, Args&&...args)
	{
		static_assert(std::is_base_of<Component<T>, T>::value,
			"Component to add must derive from base class 'Component<T>'" > );

		ComponentTypeId new_component_type = Component<T>::GetTypeId();
		if (!new_component_type) {
			// We are adding a component of this type for the first time. Register it.
			new_component_type = RegisterComponent<T>();
		}

		T new_component;
		if (entity_archetype_record_map_.find(entity_id) != entity_archetype_record_map_.end())
		{
			Record record = entity_archetype_record_map_[entity_id];
			Archetype *previous_archetype = record.archtype_ptr;
			if (std::find(previous_archetype->componentTypes.begin(),
				previous_archetype->componentTypes.end(),
				new_component_type) != previous_archetype->componentTypes.end())
			{
				throw std::runtime_error("Cannot have multiple components of same type on entity");
				return nullptr;
			}

			// Determine the entity's new archetype id.
			std::vector<ComponentTypeId> new_component_types;
			bool has_added_new_component = false;
			for (std::size_t i = 0; i < previous_archetype->componentTypes.size; ++i) {
				if (new_component_type < previous_archetype->componentTypes[i] &&
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
				existing_archetype->componentTypes = new_component_types;
				for (ArchetypeId::iterator it = new_component_types.begin(); it != new_component_types.end(); ++it) {
					ComponentBase *const component_base = type_component_base_map_[*it];
					existing_archetype->componentArrays.push_back(new char[component_base->ComponentSize()]);
					existing_archetype->componentArrayAllocSizes.push_back(component_base->ComponentSize());
				}
			}

			std::size_t entity_count = existing_archetype->entityIds.size;
			// Resize existing archetype's component arrays if necessary to fit new entity's data
			for (std::size_t c_idx = 0; c_idx < existing_archetype->componentTypes.size; ++c_idx) {
				ComponentBase *const component_base = type_component_base_map_[existing_archetype->componentTypes[c_idx]];

				std::size_t new_data_size = (entity_count + 1) * component_base->ComponentSize();
				std::size_t new_alloc_size = new_data_size;
				if (new_data_size > existing_archetype->componentArrayAllocSizes[c_idx]) {
					new_alloc_size *= 2;
				}
				else {
					// No need to resize this component array
					continue;
				}

				char *component_array = new char[new_alloc_size];
				for (std::size_t e_idx = 0; e_idx < entity_count; ++e_idx) {
					void *component_source = &existing_archetype->componentArrays[c_idx][e_idx * component_base->ComponentSize()];
					component_base->CopyData(component_source, &component_array[e_idx * component_base->ComponentSize()]);
					component_base->DeleteData(component_source);
				}
				delete[] existing_archetype->componentArrays[i];
				existing_archetype->componentArrays[i] = component_array;
				existing_archetype->componentArrayAllocSizes[i] = new_alloc_size;
			}

			// Copy over the entity's component data from the previous archetype to
			// the existing one and insert new component data.
			ComponentBase *const component_base = type_component_base_map_[new_component_type];
			for (std::size_t c_idx = 0; c_idx < existing_archetype->componentTypes.size; ++c_idx) {
				if (existing_archetype->componentTypes[c_idx] == new_component) {
					new_component = new (&existing_archetype->componentArrays[c_idx][record.index * sizeof(T)]) T();
				}
				else if (existing_archetype->componentTypes[c_idx] > new_component) {
					void *previous_component_source = &previous_archetype->componentArrays[c_idx - 1][record.index * component_base->ComponentSize()];
					componentBase->CopyData(previous_component_source, &existing_archetype->componentArrays[c_idx - 1][record.index * component_base->ComponentSize()]);
				}
				else {
					void *previous_component_source = &previous_archetype->componentArrays[c_idx][record.index * component_base->ComponentSize()];
					componentBase->CopyData(previous_component_source, &existing_archetype->componentArrays[c_idx][record.index * component_base->ComponentSize()]);
				}
			}

			if (record.index < (previous_archetype->entityIds.size - 1)) {
				// For previous archetype, move the end entity's component data to current entity's previous spot
				// This is a "fast" way of deleting the entity's component data without resizing array
				for (std::size_t c_idx = 0; c_idx < previous_archetype->componentTypes.size; +c_idx) 
				{
					componentBase->CopyData(&previous_archetype->componentArrays[c_idx][previous_archetype->entityIds.size * component_base->ComponentSize()], previous_component_source);
					componentBase->DeleteData(&previous_archetype->componentArrays[c_idx][previous_archetype->entityIds.size * component_base->ComponentSize()]);
				}
				previous_archetype->entityIds[record.index] = previous_archetype->entityIds.end;
				Record new_end_record =
				{
					previous_archetype,
					record.index
				};
				entity_archetype_record_map_.insert({ previous_archetype->entityIds.end, new_end_record });
			}
			previous_archetype->entityIds.pop_back();
			if (previous_archetype->entityIds.size == 0) {
				archetypes_.erase(previous_archetype);
			}

			Record new_record =
			{
				existing_archetype,
				entity_count
			};
			existing_archetype->entityIds.push_back(entity_id);
			entity_archetype_record_map_.insert({ entity_id, new_record });
		}
		else {
			Archetype *existing_archetype = GetMatchingArchetype({ new_component_type });
			if (existing_archetype) {
				// Add entity to existing archetype
				std::size_t entity_count = existing_archetype->entityIds.size;
				std::size_t new_data_size = (entity_count + 1) * sizeof(T);
				std::size_t new_alloc_size = new_data_size;
				if (new_data_size > existing_archetype->componentArrayAllocSizes[0]) {
					new_alloc_size *= 2;
				}
				char *componentArray = new char[new_alloc_size];
				ComponentBase *const componentBase = type_component_base_map_[new_component_type];
				for (std::size_t i = 0; i < entity_count; ++i) {
					void *componentSource = &existing_archetype->componentArrays[0][i * sizeof(T)];
					componentBase->CopyData(componentSource, &componentArray[i * sizeof(T)]);
					componentBase->DeleteData(componentSource);
				}
				new_component = new (&componentArray[entity_count * sizeof(T)]) T();
				delete[] existing_archetype->componentArrays[0];
				existing_archetype->componentArrays[0] = componentArray;
				existing_archetype->entityIds.push_back(entity_id);
				existing_archetype->componentArrayAllocSizes[0] = new_alloc_size;
				Record newRecord =
				{
					existing_archetype,
					entity_count
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
			}
			else {
				// Create new archetype for entity.
				Archetype *newArchetype = new Archetype();
				newArchetype->componentTypes = { new_component_type };
				newArchetype->componentArrays = { new char[sizeof(T)] };
				new (&newArchetype->componentArrays[0]) T();
				newArchetype->componentArrayAllocSizes = sizeof(T);
				archetypes_.push_back(newArchetype);
				Record newRecord =
				{
					newArchetype,
					0
				};
				entity_archetype_record_map_.insert({ entity_id, newRecord });
			}
		}

		Component<T>::count += 1;
		return new_component;
	}
	*/

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
	T& GetComponent(EntityId entity_id)
	{
		Record record = entity_archetype_record_map_[entity_id];
		std::size_t component_count = record.archtype_ptr->componentTypes.size;
		ComponentTypeId component_type = Component<T>::GetTypeId();
		for (int i = 0; i < component_count; i++) {
			if (component_type == record.archetype_ptr->componentTypes[i]) {
				return (T)record.archtype_ptr->componentArrays[i][record.index];
			}
		}
		return NULL;
	}

private:
	struct Record {
		Archetype *archtype_ptr;
		std::size_t index;
	};

	std::vector<Archetype *> archetypes_;
	std::unordered_map<EntityId, Record> entity_archetype_record_map_;
	std::unordered_map<ComponentTypeId, ComponentBase*> type_component_base_map_;

	Archetype *FindMatchingArchetype(ArchetypeId archtype_Id)
	{
		for (std::vector<Archetype>::iterator it = archetypes_.begin(); it != archetypes_.end(); ++it) {
			if (archtype_Id == it->componentTypes) {
				return &(*it);
			}
		}
		return nullptr;
	}

	template<typename T>
	T* FindComponentArray(ArchetypeId archetype_Id) 
	{
		ComponentTypeId component_type = Component<T>::GetTypeId();
		// TODO: do binary search to look for component array

		//if not found
		return nullptr
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