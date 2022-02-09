#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <core/utils/type_info.h>

#include "component_array.h"
#include "entity.h"

namespace ecs {
	typedef uint32_t ComponentTypeID;
	// Always in order Least->Greatest
	typedef std::vector<ComponentTypeID> ArchetypeId;

	class Archetype
	{
	public:

		Archetype() = delete;

		Archetype(TypeInfo* component_type_info) {
			component_type_info_ = component_type_info;
		}

		~Archetype() {
			for (ComponentArrayBase* component_array : component_arrays_) {
				delete component_array;
			}
		}

		template<class... Ts>
		static Archetype* ArchetypeWithComponentTypes(TypeInfo* component_type_info)
		{
			std::vector<ComponentTypeID> unsorted_component_types = { (component_type_info->GetTypeId<Ts>())... };
			std::vector<ComponentArrayBase*> unsorted_component_arrays = { (new ComponentArray<Ts>())... };
			std::vector<std::pair<ComponentTypeID, ComponentArrayBase*>> sorted_component_type_array_pairs;
			sorted_component_type_array_pairs.reserve(unsorted_component_types.size());
			for (std::size_t index = 0; index < unsorted_component_types.size(); ++index) {
				sorted_component_type_array_pairs.push_back(std::make_pair(unsorted_component_types[index], unsorted_component_arrays[index]));
			}
			std::sort(sorted_component_type_array_pairs.begin(), sorted_component_type_array_pairs.end());

			Archetype* new_archetype = new Archetype(component_type_info);
			new_archetype->component_types_.reserve(sorted_component_type_array_pairs.size());
			new_archetype->component_arrays_.reserve(sorted_component_type_array_pairs.size());
			for (std::size_t index = 0; index < sorted_component_type_array_pairs.size(); ++index) {
				new_archetype->component_type_index_map_.insert(std::make_pair(sorted_component_type_array_pairs[index].first, index));
				new_archetype->component_types_.push_back(sorted_component_type_array_pairs[index].first);
				new_archetype->component_arrays_.push_back(sorted_component_type_array_pairs[index].second);
			}
			return new_archetype;
		}

		template<class T>
		Archetype* EmptyWithAddedComponentType(TypeInfo* component_type_info)
		{
			const ComponentTypeID added_component_type = component_type_info->GetTypeId<T>();
			Archetype* new_archetype = new Archetype(component_type_info);
			new_archetype->component_types_.reserve(component_types_.size() + 1);
			new_archetype->component_arrays_.reserve(component_arrays_.size() + 1);
			auto insert_component_array = [new_archetype](ComponentArrayBase* comp_array, ComponentTypeID component_type) {
				std::size_t index = new_archetype->component_types_.size();
				new_archetype->component_type_index_map_.insert(std::make_pair(component_type, index));
				new_archetype->component_types_.push_back(component_type);
				new_archetype->component_arrays_.push_back(comp_array);
			};

			bool has_inserted_added_component = false;
			for (std::size_t c_idx = 0; c_idx < component_types_.size(); ++c_idx) {
				if (added_component_type < component_types_[c_idx] && !has_inserted_added_component) {
					insert_component_array(new ComponentArray<T>(), added_component_type);
					has_inserted_added_component = true;
				}
				insert_component_array(component_arrays_[c_idx]->Empty(), component_types_[c_idx]);
			}
			if (!has_inserted_added_component) {
				insert_component_array(new ComponentArray<T>(), added_component_type);
			}
			return new_archetype;
		}

		template<class T>
		Archetype* EmptyWithRemovedComponentType(TypeInfo* component_type_info)
		{
			const ComponentTypeID removed_component_type = component_type_info->GetTypeId<T>();
			Archetype* new_archetype = new Archetype(component_type_info);
			new_archetype->component_types_.reserve(component_types_.size() - 1);
			new_archetype->component_arrays_.reserve(component_arrays_.size() - 1);
			for (std::size_t c_idx = 0; c_idx < component_types_.size(); ++c_idx) {
				if (component_types_[c_idx] != removed_component_type) {
					new_archetype->component_type_index_map_.insert(std::make_pair(component_types_[c_idx], new_archetype->component_types_.size()));
					new_archetype->component_types_.push_back(component_types_[c_idx]);
					new_archetype->component_arrays_.push_back(component_arrays_[c_idx]->Empty());
				}
			}
			return new_archetype;
		}

		template<class T>
		void MoveEntityToSuperArchetype(EntityID entity_id, Archetype& super_archetype, T added_component)
		{
			std::size_t index = entity_components_index_map_[entity_id.index];
			ComponentTypeID added_component_type = component_type_info_->GetTypeId<T>();
			for (std::size_t c_idx = 0; c_idx < super_archetype.component_types_.size(); ++c_idx) {
				if (super_archetype.component_types_[c_idx] == added_component_type) {
					ComponentArray<T>* casted_existing_component_array = static_cast<ComponentArray<T> *>(super_archetype.component_arrays_[c_idx]);
					casted_existing_component_array->Append(added_component);
				}
				else if (super_archetype.component_types_[c_idx] > added_component_type) {
					super_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx - 1], index);
					component_arrays_[c_idx - 1]->RemoveWithSwapAtIndex(index);
				}
				else {
					super_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx], index);
					component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
				}
			}
			super_archetype.entity_components_index_map_.insert(std::make_pair(entity_id.index, super_archetype.entity_ids_.size()));
			super_archetype.entity_ids_.push_back(entity_id);
			entity_components_index_map_.erase(entity_id.index);
			if (index < entity_ids_.size() - 1) {
				entity_components_index_map_[entity_ids_.back().index] = index;
				entity_ids_[index] = entity_ids_.back();
			}
			entity_ids_.pop_back();
		}

		template<class T>
		void MoveEntityToSubArchetype(EntityID entity_id, Archetype& sub_archetype)
		{
			std::size_t index = entity_components_index_map_[entity_id.index];
			ComponentTypeID removed_component_type = component_type_info_->GetTypeId<T>();
			for (std::size_t c_idx = 0; c_idx < component_arrays_.size(); ++c_idx) {
				if (component_types_[c_idx] == removed_component_type) {
					component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
				}
				else if (component_types_[c_idx] > removed_component_type) {
					sub_archetype.component_arrays_[c_idx - 1]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx], index);
					component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
				}
				else {
					sub_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx], index);
					component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
				}
			}
			sub_archetype.entity_ids_.push_back(entity_ids_[index]);
			entity_components_index_map_.erase(entity_id.index);
			if (index < entity_ids_.size() - 1) {
				entity_components_index_map_[entity_ids_.back().index] = index;
				entity_ids_[index] = entity_ids_.back();
			}
			entity_ids_.pop_back();
		}

		template<class... Ts>
		void AddEntity(EntityID entity_id, Ts ...components)
		{
			std::vector<ComponentTypeID> added_component_types = { (component_type_info_->GetTypeId<Ts>())... };
			std::sort(added_component_types.begin(), added_component_types.end());
			if (added_component_types != component_types_) {
				throw std::runtime_error("Number and order of specified components must match that of Archetype.");
			}

			AddComponents<Ts...>(components...);
			entity_components_index_map_.insert(std::make_pair(entity_id.index, entity_ids_.size()));
			entity_ids_.push_back(entity_id);
		}

		void RemoveEntity(EntityID entity_id)
		{
			const std::size_t index = entity_components_index_map_[entity_id.index];
			for (std::size_t c_idx = 0; c_idx < component_arrays_.size(); ++c_idx) {
				component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
			}
			entity_components_index_map_.erase(entity_id.index);
			if (index < entity_ids_.size() - 1) {
				entity_components_index_map_[entity_ids_.back().index] = index;
				entity_ids_[index] = entity_ids_.back();
			}
			entity_ids_.pop_back();
		}

		const ArchetypeId& ComponentTypes() {
			return component_types_;
		}

		const std::vector<EntityID>& Entities() {
			return entity_ids_;
		}

		template<class T>
		T* GetComponentForEntity(EntityID entity_id)
		{
			ComponentArray<T>* const component_array = FindComponentArray<T>();
			if (!component_array) {
				return nullptr;
			}
			const std::size_t index = entity_components_index_map_[entity_id.index];
			return &component_array->ComponentAtIndex(index);
		}

		template<class T>
		void GetComponentForEntityWithSafeBlock(EntityID entity_id, std::function<void(const T&)> success_block, std::function<void()> failure_block)
		{
			ComponentArray<T>* const component_array = FindComponentArray<T>();
			if (!component_array) {
				failure_block();
				return;
			}
			const std::size_t index = entity_components_index_map_[entity_id.index];
			success_block(component_array->ComponentAtIndex(index));
		}

		template<class... Ts>
		void EnumerateComponentsWithBlock(std::function<void(EntityID entity_id, Ts&...)> block)
		{
			[block](std::vector<EntityID>& entity_ids, ComponentArray<Ts>* ...queried_component_arrays) {
				for (std::size_t e_idx = 0; e_idx < entity_ids.size(); ++e_idx) {
					block(entity_ids[e_idx], queried_component_arrays->ComponentAtIndex(e_idx)...);
				}
			}(entity_ids_, FindComponentArray<Ts>()...);
		}

	private:
		ArchetypeId component_types_;

		std::vector<ComponentArrayBase*> component_arrays_;

		std::vector<EntityID> entity_ids_;

		std::unordered_map<std::uint32_t, std::size_t> entity_components_index_map_;

		std::unordered_map<ComponentTypeID, std::size_t> component_type_index_map_;

		TypeInfo* component_type_info_;

		template<class T>
		ComponentArray<T>* FindComponentArray()
		{
			const ComponentTypeID component_type = component_type_info_->GetTypeId<T>();
			const std::unordered_map<ComponentTypeID, std::size_t>::iterator component_array_index_iter = component_type_index_map_.find(component_type);
			if (component_array_index_iter == component_type_index_map_.end()) {
				return nullptr;
			}
			return static_cast<ComponentArray<T> *>(component_arrays_[component_array_index_iter->second]);
		}

		template<class... Ts>
		void AddComponents(Ts... components);

		template<>
		void AddComponents() {}

		template<class T, class... Ts>
		void AddComponents(T component, Ts... components) {
			ComponentArray<T>* component_array = FindComponentArray<T>();
			component_array->Append(component);
			AddComponents<Ts...>(components...);
		}
	};

}


