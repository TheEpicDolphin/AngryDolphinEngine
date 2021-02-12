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
};

template<class T>
class ComponentArray : ComponentArrayBase 
{
public:
	void Append(ComponentBase component) {
		T casted_component = static_cast<T>(component)
		components.push_back(casted_component);
	}

	ComponentBase RemoveWithSwapAtIndex(std::size_t index)
	{
		if (index >= components.size) {
			return NULL;
		}
		T component = components[index];
		if (index < components.size - 1) {
			components[index] = components.end();
		}
		components.pop_back();
		return (ComponentBase)component;
	}

	void ReadComponentAtIndex(std::size_t index, std::function<void(const T&)> read_block) 
	{
		if (index >= components.size) {
			read_block(NULL);
		}
		else {
			read_block(components[index]);
		}
	}

	void UpdateComponentAtIndex(std::size_t index, std::function<void(T&)> write_block) 
	{

	}

private:
	std::vector<T> components;
};

// Always in order Least->Greatest
typedef std::vector<ComponentTypeId> ArchetypeId;

class Archetype 
{
public:
	public Archetype(ArchetypeId component_types) {
		this->componentTypes = component_types;
	}

	ArchetypeId componentTypes;
	
	std::vector<void *> componentArrays;

	std::vector<std::size_t> componentArrayAllocSizes;

	std::vector<EntityId> entityIds;

	std::vector<ComponentArrayBase*> component_arrays;

	template<class T>
	ComponentArray<T>* GetComponentArray() 
	{
		ComponentTypeId component_type = Component<T>::GetTypeId();
		for (std::size_t c_idx = 0; c_idx < componentTypes.size; ++c_idx) {
			if (component_type == componentTypes[c_idx]) {
				return static_cast<T*>(component_arrays[c_idx]);
			}
		}
		return nullptr;
	}
};
