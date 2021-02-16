#pragma once

#include <iostream>
#include <vector>
#include <functional>

#include "type_id_generator.h"
#include "component.h"
#include "entity.h"

class ComponentArrayBase
{
public:
	virtual void Append(ComponentBase component) {}

	virtual ComponentBase RemoveWithSwapAtIndex(std::size_t index) {}

	virtual ComponentArrayBase* Empty() {}
};

template<class T>
class ComponentArray : ComponentArrayBase 
{
public:
	void Append(ComponentBase component) {
		T casted_component = static_cast<T>(component)
		components_.push_back(casted_component);
	}

	ComponentBase RemoveWithSwapAtIndex(std::size_t index)
	{
		if (index >= components_.size) {
			throw std::runtime_error("ComponentArray index out of range.");
		}
		T component = components_[index];
		if (index < components_.size - 1) {
			components_[index] = components_.end();
		}
		components_.pop_back();
		return (ComponentBase)component;
	}

	ComponentArrayBase* Empty() {
		return new ComponentArray<T>();
	}

	T& ComponentAtIndex(std::size_t index)
	{
		if (index >= components_.size) {
			throw std::runtime_error("ComponentArray index out of range.");
		}
		else {
			return components_[index];
		}
	}

private:
	std::vector<T> components_;
};

// Always in order Least->Greatest
typedef std::vector<ComponentTypeId> ArchetypeId;

class Archetype 
{
public:
	ArchetypeId component_types;

	std::vector<EntityId> entity_ids;

	std::vector<ComponentArrayBase*> component_arrays;

	template<class T>
	ComponentArray<T>* GetComponentArray() 
	{
		ComponentTypeId component_type = Component<T>::GetTypeId();
		// TODO: Perform binary search to look for component array given component_type
		for (std::size_t c_idx = 0; c_idx < componentTypes.size; ++c_idx) {
			if (component_type == componentTypes[c_idx]) {
				return static_cast<T*>(component_arrays[c_idx]);
			}
		}
		return nullptr;
	}
};
