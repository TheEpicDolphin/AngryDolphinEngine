#pragma once

#include <stdexcept>
#include <vector>
#include <config/engine_components.h>

namespace ecs {
	enum ComponentArrayType
	{
#define REGISTER_COMPONENT(name) nameComponentArrayType,
		REGISTERED_ENGINE_COMPONENTS
		//REGISTERED_PROJECT_COMPONENTS
#undef REGISTER_COMPONENT
	};

class ComponentArrayBase
	{
	public:
		virtual void AppendComponentFromArrayAtIndex(ComponentArrayBase* source_component_array, std::size_t index) {}

		virtual void RemoveWithSwapAtIndex(std::size_t index) {}

		virtual ComponentArrayBase* Empty()
		{
			return nullptr;
		}

		ComponentArrayBase* DynamicallyAllocatedDerivedObject()//rapidxml::xml_node<>& xml_node)
		{
//			switch () {
//#define REGISTER_COMPONENT(name) case nameComponentArrayType: \
//				return new ComponentArray<name>();
//				REGISTERED_ENGINE_COMPONENTS
//#undef REGISTER_COMPONENT
//			}
			return nullptr;
		}
	};

	template<class T>
	class ComponentArray : public ComponentArrayBase
	{
	public:
		ComponentArray() {}

		void Append(T component)
		{
			components_.push_back(component);
		}

		void AppendComponentFromArrayAtIndex(ComponentArrayBase* source_component_array, std::size_t index) override
		{
			ComponentArray<T>* casted_source_component_array = static_cast<ComponentArray<T> *>(source_component_array);
			Append(casted_source_component_array->ComponentAtIndex(index));
		}

		void RemoveWithSwapAtIndex(std::size_t index) override
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

		ComponentArrayBase* Empty() override {
			return new ComponentArray<T>();
		}

		T& ComponentAtIndex(std::size_t index) {
			return components_[index];
		}

	private:
		std::vector<T> components_;
	};
}
