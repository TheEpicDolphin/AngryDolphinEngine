#pragma once

#include <iostream>
#include <vector>

#include "type_id_generator.h"
#include "component.h"
#include "entity.h"

struct ComponentArrayBase
{
public:
	virtual void Append(ComponentBase component) {}

	virtual ComponentBase RemoveWithSwapAtIndex(std::size_t index) {}

	// TODO: This is sketchy because pointer can be invalidated when vector is resized
	virtual ComponentBase* ItemAtIndex(std::size_t index) {}
};

template<class T>
struct ComponentArray : ComponentArrayBase 
{
	std::vector<T> components;

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

	ComponentBase* ItemAtIndex(std::size_t index) 
	{
		if (index >= components.size) {
			return nullptr;
		}
		return &components.data[index];
	}
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
};
