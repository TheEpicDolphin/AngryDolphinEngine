#pragma once

#include <iostream>
#include <vector>
#include <functional>

#include "component.h"
#include "entity.h"

class ComponentArrayBase
{
public:
	virtual void AppendComponentFromArrayAtIndex(ComponentArrayBase* source_component_array, std::size_t index) {}

	virtual void RemoveWithSwapAtIndex(std::size_t index) {}

	virtual ComponentArrayBase* Empty() {}
};

template<class T>
class ComponentArray : public ComponentArrayBase
{
public:
	void Append(T component) 
	{
		components_.push_back(component);
	}

	void AppendComponentFromArrayAtIndex(ComponentArrayBase* source_component_array, std::size_t index)
	{
		ComponentArray<T>* casted_source_component_array = static_cast<ComponentArray<T> *>(source_component_array);
		Append(casted_source_component_array->ComponentAtIndex(index));
	}

	void RemoveWithSwapAtIndex(std::size_t index)
	{
		if (index >= components_.size()) {
			throw std::runtime_error("ComponentArray index out of range.");
		}
		T component = components_[index];
		if (index < components_.size() - 1) {
			components_[index] = components_.back();
		}
		components_.pop_back();
	}

	ComponentArrayBase* Empty() {
		return new ComponentArray<T>();
	}

	T& ComponentAtIndex(std::size_t index) {
		return components_[index];
	}

private:
	std::vector<T> components_;
};

// Always in order Least->Greatest
typedef std::vector<ComponentTypeID> ArchetypeId;

class Archetype 
{
private:
	ArchetypeId component_types_;

	// TODO: Consider replacing this with simple array
	std::vector<ComponentArrayBase*> component_arrays_;

public:

	Archetype(ArchetypeId component_types) {
		component_types_ = component_types;
		component_arrays_.resize(component_types.size());
	}

	~Archetype() {
		for (ComponentArrayBase *component_array : component_arrays_) {
			delete component_array;
		}
	}

	template<class T>
	void MoveEntityAtIndexToSuperArchetype(std::size_t index, Archetype& super_archetype, T added_component) 
	{
		ComponentTypeID added_component_type = Component<T>::GetTypeId();
		for (std::size_t c_idx = 0; c_idx < component_types_.size(); ++c_idx) {
			if (super_archetype.component_arrays_[c_idx] == added_component_type) {
				ComponentArray<T>* casted_existing_component_array = static_cast<ComponentArray<T> *>(super_archetype.component_arrays_[c_idx]);
				casted_existing_component_array->Append(added_component);
			}
			else if (existing_archetype.component_types_[c_idx] > added_component_type) {
				super_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx - 1], index);
				component_arrays_[c_idx - 1]->RemoveWithSwapAtIndex(record.index);
			}
			else {
				super_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx], index);
				component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
			}
		}
		super_archetype.entity_ids.push_back(entity_ids[index]);
		entity_ids[index] = entity_ids.back();
		entity_ids.pop_back();
	}

	template<class T>
	void MoveEntityAtIndexToSubArchetype(std::size_t index, Archetype& sub_archetype)
	{
		ComponentTypeID removed_component_type = Component<T>::GetTypeId();
		for (std::size_t c_idx = 0; c_idx < component_arrays_.size(); ++c_idx) {
			if (component_types_[c_idx] == removed_component_type) {
				component_arrays_[c_idx]->RemoveWithSwapAtIndex(record.index);
			}
			else if (component_types_[c_idx] > removed_component_type) {
				existing_archetype->component_arrays[c_idx - 1]->AppendComponentFromArrayAtIndex(component_arrays_[c_idx], index);
				previous_archetype->component_arrays[c_idx]->RemoveWithSwapAtIndex(index);
			}
			else {
				sub_archetype.component_arrays_[c_idx]->AppendComponentFromArrayAtIndex(previous_archetype->component_arrays[c_idx], index);
				component_arrays_[c_idx]->RemoveWithSwapAtIndex(index);
			}
		}
		super_archetype.entity_ids.push_back(entity_ids[index]);
		entity_ids[index] = entity_ids.back();
		entity_ids.pop_back();
	}

	std::vector<EntityID> entity_ids;

	ArchetypeId ComponentTypes() {
		return component_types_;
	}

	template<class T>
	ComponentArray<T>* GetComponentArray() 
	{
		ComponentTypeID component_type = Component<T>::GetTypeId();
		// TODO: Perform binary search to look for component array given component_type
		for (std::size_t c_idx = 0; c_idx < component_types_.size(); ++c_idx) {
			if (component_type == component_types_[c_idx]) {
				return static_cast<ComponentArray<T> *>(component_arrays_[c_idx]);
			}
		}
		return nullptr;
	}
};
