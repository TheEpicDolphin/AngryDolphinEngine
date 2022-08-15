#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <stack>
#include <queue>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "variable.h"
#include "serdes_utils.h"

class XMLSerializer 
{
public:
    // TODO: Allow serializing/deserializing functions (member & static)

	// TODO insert version numbers at beginning of xml

	XMLSerializer() {}

	~XMLSerializer() {}

	template<typename T>
	void SerializeToXML(rapidxml::xml_document<>& xml_doc, const char* name, T& root_object)
	{
		variables_ = { nullptr };
		objects_ = { nullptr };
		SerializeToXML(xml_doc, &xml_doc, serialize::CreateSerializerNode(root_object));
		variables_.clear();
	}

	template<typename T>
	void DeserializeFromXML(rapidxml::xml_document<>& xml_doc, T& root_object)
	{
		variables_ = { nullptr };
		objects_ = { nullptr };
		rapidxml::xml_document<>* root_xml_node = xml_doc.first_node();
		DeserializeFromXML(root_xml_node, serialize::CreateVariable(std::string(root_xml_node->name()), root_object));
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc.last_node("dynamic_memory");
		rapidxml::xml_node<>* dynamic_memory_child_xml_node = dynamic_memory_xml_node->first_node();
		while (!pointee_id_to_des_pointer_vars_map_.empty()) {
			std::unordered_map<std::size_t, std::vector<std::shared_ptr<IPointerVariable>>>::iterator first_node_pair_iter = pointee_id_to_des_pointer_vars_map_.begin();
			const std::size_t var_id = first_node_pair_iter->first;

			std::shared_ptr<IPointerVariable> dynamic_memory_pointer_var = first_node_pair_iter->second.front();
			std::string type_name = dynamic_memory_pointer_var->TypeName();
			dynamic_memory_pointer_var->SetValue(serialize::rt_type_registry.InstantiateObjectFromTypeName(type_name));
			std::shared_ptr<IVariable> referenced_var = dynamic_memory_pointer_var->ReferencedVariable();
			pointee_id_to_des_pointer_vars_map_.erase(var_id);

			DeserializeFromXML(dynamic_memory_child_xml_node, referenced_var);

			dynamic_memory_child_xml_node = dynamic_memory_child_xml_node->next_sibling();
		}

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveDesNodeBase*>::reverse_iterator it = des_nodes_.rbegin(); it != std::prev(des_nodes_.rend()); ++it) {
			ArchiveDesNodeBase* node = *it;
			node->ConstructFromDeserializedDependencies();
			delete node;
		}

		variables_.clear();
		objects_.clear();
	}

	// Assumes no dynamically allocated objects within root_object.
	template<typename T>
	void DeserializeFromXML(rapidxml::xml_node<>& root_node, T& root_object)
	{
		pointee_id_to_des_pointer_vars_map_ = {};
		variables_ = { nullptr };
		objects_ = { nullptr };

		DeserializeFromXML(&root_node, serialize::CreateVariable(std::string(root_node->name()), root_object));

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveDesNodeBase*>::reverse_iterator it = des_nodes_.rbegin(); it != std::prev(des_nodes_.rend()); ++it) {
			ArchiveDesNodeBase* node = *it;
			node->ConstructFromDeserializedDependencies();
			delete node;
		}

		pointee_id_to_des_pointer_nodes_map_.clear();
		variables_.clear();
		objects_.clear();
	}

private:
	std::unordered_map<void*, std::size_t> object_to_id_map_;
	std::vector<void*> objects_;
	// BFS order
	std::vector<std::shared_ptr<IVariable>> variables_;
	std::unordered_map<std::size_t, std::vector<std::shared_ptr<INonOwningPointerVariable>>> non_owning_pointer_var_des_map_;

	std::size_t IdForObject(void* object_ptr)
	{
		if (object_to_id_map_.find(object_ptr) != object_to_id_map_.end()) {
			return object_to_id_map_[object_ptr];
		}
		return 0;
	}

	void* ObjectForId(std::size_t id) 
	{
		if (id < objects_.size()) {
			return objects_[id];
		}
		return nullptr;
	}

	std::size_t StoreObject(void* object)
	{
		const std::size_t id = objects_.size();
		object_to_id_map_[object] = id;
		objects_.push_back(object);
		return id;
	}

	std::size_t IdForXMLNode(rapidxml::xml_node<>& xml_node) 
	{
		char* id_string = xml_node.first_attribute("id")->value();
		return serialize::DeserializeArithmeticFromString<std::size_t>(id_string);
	}

	std::size_t PointeeIdForXMLNode(rapidxml::xml_node<>& xml_node)
	{
		char* pointee_id_string = xml_node.first_node("pointee")->first_attribute("pointee_id")->value();
		return serialize::DeserializeArithmeticFromString<std::size_t>(pointee_id_string);
	}

	void SerializeToXML(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_parent_xml_node, std::shared_ptr<IVariable> root_node)
	{
		std::unordered_map<std::size_t, rapidxml::xml_node<>*> child_id_to_parent_xml_node_map;
		std::unordered_map<void*, std::vector<rapidxml::xml_node<>*>> pointer_xml_node_map_;
		std::queue<std::shared_ptr<IVariable>> bfs_queue;
		std::size_t root_id = StoreObject(root_node->ObjectHandle());
		child_id_to_parent_xml_node_map[root_id] = root_parent_xml_node;
		bfs_queue.push(root_node);
		while (!bfs_queue.empty()) {
			std::shared_ptr<IVariable> var = bfs_queue.front();
			bfs_queue.pop();

			// Map object handle to a unique id for use in serialization.
			void* object_handle = var->ObjectHandle();
			std::size_t object_id = IdForObject(object_handle);

			// Notify listeners for this object's address
			std::unordered_map<void*, std::vector<rapidxml::xml_node<>*>>::iterator iter = pointer_xml_node_map_.find(object_handle);
			if (iter != pointer_xml_node_map_.end()) {
				const std::vector<rapidxml::xml_node<>*> pointer_xml_nodes = iter->second;
				pointer_xml_node_map_.erase(object_handle);
				for (rapidxml::xml_node<>* pointer_xml_node : pointer_xml_nodes) {
					std::string pointed_object_id_string = serialize::SerializeArithmeticToString(object_id);
					pointer_xml_node->value(xml_doc.allocate_string(pointed_object_id_string.c_str()));
				}
			}

			// Serialize node.
			// We must allocate a copy of the name_ c-string using xml_doc.allocate_string. When this class
			// instance is destroyed, name_ is destroyed, and so is the const char* pointer to its c-string.
			char* name_string = xml_doc.allocate_string(var->Name().c_str());
			rapidxml::xml_node<>* object_xml_node = xml_doc.allocate_node(rapidxml::node_element, name_string);
			std::size_t object_id = IdForObject(var->ObjectHandle());
			char* object_id_string = xml_doc.allocate_string(std::to_string(object_id).c_str());
			rapidxml::xml_attribute<>* object_id_atttribute = xml_doc.allocate_attribute("id", object_id_string);
			object_xml_node->append_attribute(object_id_atttribute);

			switch (var->TypeCategory()) {
			case VariableTypeCategory::Arithmetic: {
				std::shared_ptr<IArithmeticVariable> arithmetic_var = std::static_pointer_cast<IArithmeticVariable>(var);
				std::string value_as_string = arithmetic_var->ReadValueAsString();
				object_xml_node->value(xml_doc.allocate_string(value_as_string.c_str()));
				break;
			}
			case VariableTypeCategory::Enum: {
				std::shared_ptr<IEnumVariable> enum_var = std::static_pointer_cast<IEnumVariable>(var);
				int enum_value = enum_var->GetValue();
				std::string value_as_string = serialize::SerializeArithmeticToString(enum_value);
				object_xml_node->value(xml_doc.allocate_string(value_as_string.c_str()));
				break;
			}
			case VariableTypeCategory::NonOwningPointerVariable: {
				std::shared_ptr<INonOwningPointerVariable> non_owning_pointer_var = std::static_pointer_cast<INonOwningPointerVariable>(var);
				// pointee_id might be zero if the pointee hasn't been registered yet.
				void* pointed_object_handle = non_owning_pointer_var->PointedObjectHandle();
				std::size_t pointed_object_id = IdForObject(pointed_object_handle);
				rapidxml::xml_node<>* pointer_xml_node = xml_doc.allocate_node(rapidxml::node_element, "non_owning_ptr");
				object_xml_node->append_node(pointer_xml_node);

				if (pointed_object_id) {
					// Valid pointee_id. Set it as the value for this non_owning_ptr node.
					std::string pointed_object_id_string = serialize::SerializeArithmeticToString(pointed_object_id);
					pointer_xml_node->value(xml_doc.allocate_string(pointed_object_id_string.c_str()));
				}
				else {
					// The object pointed to by pointee_handle has not been registered yet.
					if (pointer_xml_node_map_.find(pointed_object_handle) != pointer_xml_node_map_.end()) {
						pointer_xml_node_map_[pointed_object_handle].push_back(pointer_xml_node);
					}
					else {
						pointer_xml_node_map_[pointed_object_handle] = { pointer_xml_node };
					}
				}
				break;
			}
			case VariableTypeCategory::OwningPointerVariable: {
				std::shared_ptr<IOwningPointerVariable> owning_pointer_var = std::static_pointer_cast<IOwningPointerVariable>(var);
				void* owned_object_handle = owning_pointer_var->OwnedObjectHandle();
				std::size_t owned_object_id = IdForObject(owned_object_handle);
				rapidxml::xml_node<>* owned_object_xml_node = xml_doc.allocate_node(rapidxml::node_element, "owning_ptr");
				if (!owned_object_id) {
					// First instance of a pointer to this dynamically allocated object.
					child_id_to_parent_xml_node_map[owned_object_id] = object_xml_node;
					bfs_queue.push(owning_pointer_var->OwnedVariable());
				} else {
					// Set pointed object id as value for this xml node.
					std::string owned_object_id_as_string = serialize::SerializeArithmeticToString(owned_object_id);
					owned_object_xml_node->value(xml_doc.allocate_string(owned_object_id_as_string.c_str()));
				}
			}
			case VariableTypeCategory::StaticArray: {
				std::shared_ptr<IStaticArrayVariable> static_array_var = std::static_pointer_cast<IStaticArrayVariable>(var);

				// Write attributes for array xml node such as "container_type" and "length".
				rapidxml::xml_attribute<>* container_attribute = xml_doc.allocate_attribute("container_type", "static_array");
				object_xml_node->append_attribute(container_attribute);
				char* array_length_string = xml_doc.allocate_string(std::to_string(static_array_var->Length()).c_str());
				rapidxml::xml_attribute<>* array_length_atttribute = xml_doc.allocate_attribute("length", array_length_string);
				object_xml_node->append_attribute(array_length_atttribute);

				// Add serialized node as child to parent.
				child_id_to_parent_xml_node_map[object_id]->append_node(object_xml_node);

				// Iterate children.
				std::vector<std::shared_ptr<IVariable>> elements = static_array_var->Elements();
				for (std::vector<std::shared_ptr<IVariable>>::iterator it = elements.begin(); it != elements.end(); ++it)
				{
					std::shared_ptr<IVariable> element = *it;
					std::size_t element_id = StoreObject(element->ObjectHandle());
					child_id_to_parent_xml_node_map[element_id] = object_xml_node;
					bfs_queue.push(element);
				}
				break;
			}
			case VariableTypeCategory::DynamicArray: {
				std::shared_ptr<IDynamicArrayVariable> dynamic_array_var = std::static_pointer_cast<IDynamicArrayVariable>(var);

				// Write attributes for array xml node such as "container_type" and "length".
				rapidxml::xml_attribute<>* container_attribute = xml_doc.allocate_attribute("container_type", "dynamic_array");
				object_xml_node->append_attribute(container_attribute);
				char* array_count_string = xml_doc.allocate_string(std::to_string(dynamic_array_var->Count()).c_str());
				rapidxml::xml_attribute<>* array_count_atttribute = xml_doc.allocate_attribute("count", array_count_string);
				object_xml_node->append_attribute(array_count_atttribute);

				// Add serialized node as child to parent.
				child_id_to_parent_xml_node_map[object_id]->append_node(object_xml_node);

				// Iterate children.
				std::vector<std::shared_ptr<IVariable>> elements = dynamic_array_var->Elements();
				for (std::vector<std::shared_ptr<IVariable>>::iterator it = elements.begin(); it != elements.end(); ++it)
				{
					std::shared_ptr<IVariable> element = *it;
					std::size_t element_id = StoreObject(element->ObjectHandle());
					child_id_to_parent_xml_node_map[element_id] = object_xml_node;
					bfs_queue.push(element);
				}
				break;
			}
			case VariableTypeCategory::Class: {
				std::shared_ptr<IClassVariable> class_var = std::static_pointer_cast<IClassVariable>(var);
				// Add serialized node as child to parent.
				child_id_to_parent_xml_node_map[object_id]->append_node(object_xml_node);

				// Iterate children.
				std::vector<std::shared_ptr<IVariable>> members = class_var->MemberVariables();
				for (std::vector<std::shared_ptr<IVariable>>::iterator it = members.begin(); it != members.end(); ++it)
				{
					std::shared_ptr<IVariable> member = *it;
					std::size_t member_id = StoreObject(member->ObjectHandle());
					child_id_to_parent_xml_node_map[member_id] = object_xml_node;
					bfs_queue.push(member);
				}
				break;
			}
			default:
				break;
			}
		}
	}
	
	void DeserializeFromXML(rapidxml::xml_node<>* root_xml_node, std::shared_ptr<IVariable> root_variable, bool strict_format = true)
	{
		std::queue<std::shared_ptr<IVariable>> bfs_queue;
		bfs_queue.push(root_variable);
		std::queue<rapidxml::xml_node<>*> bfs_xml_node_queue;
		bfs_xml_node_queue.push(root_xml_node);
		while (!bfs_queue.empty()) {
			std::shared_ptr<IVariable> var = bfs_queue.front();
			bfs_queue.pop();

			rapidxml::xml_node<>* xml_node = bfs_xml_node_queue.front();
			bfs_xml_node_queue.pop();

			void* object_handle = var->ObjectHandle();
			const std::size_t object_id = StoreObject(object_handle);
			if (strict_format) {
				assert(IdForXMLNode(*xml_node) == object_id);
				assert(strcmp(xml_node->name(), var->Name().c_str()));
			}

			// Notify listeners for this object's id
			std::unordered_map<std::size_t, std::vector<std::shared_ptr<IPointerVariable>>>::iterator iter = pointee_id_to_des_pointer_vars_map_.find(object_id);
			if (iter != pointee_id_to_des_pointer_vars_map_.end()) {
				const std::vector<std::shared_ptr<IPointerVariable>> pointer_listeners = iter->second;
				pointee_id_to_des_pointer_vars_map_.erase(object_id);
				for (std::shared_ptr<IPointerVariable> pointer_listener : pointer_listeners) {
					pointer_listener->SetValue(object_handle);
				}
			}

			switch (var->TypeCategory()) {
			case VariableTypeCategory::Arithmetic: {
				std::shared_ptr<IArithmeticVariable> arithmetic_var = std::static_pointer_cast<IArithmeticVariable>(var);
				arithmetic_var->SetValueFromString(xml_node->value());
				break;
			}
			case VariableTypeCategory::Enum: {
				std::shared_ptr<IEnumVariable> enum_var = std::static_pointer_cast<IEnumVariable>(var);
				int enum_value;
				serialize::DeserializeArithmeticFromString(enum_value, xml_node->value());
				enum_var->SetValue(enum_value);
				break;
			}
			case VariableTypeCategory::Pointer: {
				std::shared_ptr<IPointerVariable> pointer_var = std::static_pointer_cast<IPointerVariable>(var);
				// Get pointee id from the xml node.
				const std::size_t pointee_id = PointeeIdForXMLNode(*xml_node);

				// Attempt to get the void pointer corresponding to the pointee id.
				void* pointee_object = ObjectForId(pointee_id);
				if (pointee_object) {
					pointer_var->SetValue(pointee_object);
				}
				else {
					// The object pointed to by object_ptr has not been registered yet.
					if (pointee_id_to_des_pointer_vars_map_.find(pointee_id) != pointee_id_to_des_pointer_vars_map_.end()) {
						pointee_id_to_des_pointer_vars_map_[pointee_id].push_back(pointer_var);
					}
					else {
						pointee_id_to_des_pointer_vars_map_[pointee_id] = { pointer_var };
					}
				}
				break;
			}
			case VariableTypeCategory::Array: {
				std::shared_ptr<IArrayVariable> array_var = std::static_pointer_cast<IArrayVariable>(var);

				// Iterate children.
				std::vector<std::shared_ptr<IVariable>> elements = array_var->Elements();
				rapidxml::xml_node<>* element_xml_node = xml_node->first_node();
				for (std::vector<std::shared_ptr<IVariable>>::iterator it = elements.begin(); it != elements.end(); ++it)
				{
					std::shared_ptr<IVariable> element = *it;
					bfs_queue.push(element);
					bfs_xml_node_queue.push(element_xml_node);
					element_xml_node = element_xml_node->next_sibling();
				}
				break;
			}
			case VariableTypeCategory::Class: {
				std::shared_ptr<IClassVariable> class_var = std::static_pointer_cast<IClassVariable>(var);

				// Iterate children.
				std::vector<std::shared_ptr<IVariable>> members = class_var->MemberVariables();
				rapidxml::xml_node<>* member_xml_node = xml_node->first_node();
				for (std::vector<std::shared_ptr<IVariable>>::iterator it = members.begin(); it != members.end(); ++it)
				{
					std::shared_ptr<IVariable> member = *it;
					bfs_queue.push(member);
					bfs_xml_node_queue.push(member_xml_node);
					member_xml_node = member_xml_node->next_sibling();
				}
				break;
			}
			default:
				break;
			}
		}
	}
	
};