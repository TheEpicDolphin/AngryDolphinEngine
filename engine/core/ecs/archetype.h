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
		Append(casted_source_component_array[index]);
	}

	void RemoveWithSwapAtIndex(std::size_t index)
	{
		if (index >= components_.size) {
			throw std::runtime_error("ComponentArray index out of range.");
		}
		T component = components_[index];
		if (index < components_.size - 1) {
			components_[index] = components_.end();
		}
		components_.pop_back();
	}

	ComponentArrayBase* Empty() {
		return new ComponentArray<T>();
	}

	T& operator[](std::size_t index) {
		return components_[index];
	}

private:
	std::vector<T> components_;
};

// Always in order Least->Greatest
typedef std::vector<ComponentTypeID> ArchetypeId;

class Archetype 
{
public:
	ArchetypeId component_types;

	std::vector<EntityID> entity_ids;

	std::vector<ComponentArrayBase*> component_arrays;

	template<class T>
	ComponentArray<T>* GetComponentArray() 
	{
		ComponentTypeID component_type = Component<T>::GetTypeId();
		// TODO: Perform binary search to look for component array given component_type
		for (std::size_t c_idx = 0; c_idx < component_types.size; ++c_idx) {
			if (component_type == component_types[c_idx]) {
				return static_cast<T*>(component_arrays[c_idx]);
			}
		}
		return nullptr;
	}
};
