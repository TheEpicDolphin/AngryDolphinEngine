#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <stack>
#include <queue>
#include <set>

//#include <rapidxml/rapidxml.hpp>
//#include <rapidxml/rapidxml_print.hpp>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "serializable.h"
#include "archive_node.h"

class Archive 
{
public:

    // TODO create map from strings to functions that will be used in case pointers cannot be figured out. This allows
    // serializing/deserializing function pointers

	// TODO insert version numbers at beginning of xml

    Archive() {}

	template<typename T>
	ArchiveNodeBase* RegisterMember(const char* name, T& object)
	{
		std::unordered_map<void*, ArchiveNodeBase*>::iterator iter = dynamic_memory_candidate_nodes_map_.find(&object);
		if (iter != dynamic_memory_candidate_nodes_map_.end()) {
			ArchiveNodeBase* object_node = iter->second;
			dynamic_memory_candidate_nodes_map_.erase((void*)&object);
			return object_node;
		}

		// Important to do ++next_id_ because we want 0 to be the null id.
		ArchiveNodeBase* object_node = (ArchiveNodeBase*) new ArchiveObjectNode<T>(++next_id_, name, object);
		object_to_node_map_[(void*)&object] = object_node;
		return object_node;
	}

	template<typename T>
	ArchiveNodeBase* RegisterMember(const char* name, T*& object_ptr)
	{
		std::size_t pointee_id = IdForObject((void*)object_ptr);
		if (!pointee_id) {
			// The object pointed to by object_ptr has not been registered yet.
			ArchiveNodeBase* pointee_node = RegisterMember("object", *object_ptr);
			dynamic_memory_candidate_nodes_map_.insert({ (void*)object_ptr, pointee_node });
			pointee_id = pointee_node->Id();
		}
		// Important to do ++next_id_ because we want 0 to be the null id.
		ArchiveNodeBase* pointer_node = (ArchiveNodeBase*) new ArchivePointerNode(++next_id_, name, pointee_id);
		object_to_node_map_[(void*)&object_ptr] = pointer_node;
		return pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveNodeBase*> RegisterMembers(std::pair<const char*, Ts&>... objects)
	{
		const std::vector<ArchiveNodeBase *> child_nodes = { (RegisterMember(objects.first, objects.second))... };
		return child_nodes;
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		rapidxml::xml_document<> xml_doc;
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc.allocate_node(rapidxml::node_element, "dynamic_memory");
		next_id_ = 0;

		SerializeHumanReadableFromNode(xml_doc, &xml_doc, RegisterMember(name, root_object));

		while (!dynamic_memory_candidate_nodes_map_.empty()) {
			std::unordered_map<void*, ArchiveNodeBase*>::iterator first_node_pair_iter = dynamic_memory_candidate_nodes_map_.begin();
			ArchiveNodeBase* dynamic_memory_node = first_node_pair_iter->second;
			dynamic_memory_candidate_nodes_map_.erase(first_node_pair_iter->first);
			SerializeHumanReadableFromNode(xml_doc, dynamic_memory_xml_node, dynamic_memory_node);
		}

		xml_doc.append_node(dynamic_memory_xml_node);
		xml_ostream << xml_doc;

		for (std::pair<void*, ArchiveNodeBase*> object_node_pair : object_to_node_map_) {
			delete object_node_pair.second;
		}
		object_to_node_map_.clear();
		xml_doc.clear();
	}

	/*
	// completion is called when all the args have been retrieved from the XML file.
	template<typename T, typename... Args>
	void DeserializeHumanReadableWithCompletion(T object, std::pair<std::string, Args&> ...args, void (*completion)(T&, Args&...))
	{

	}

    template<typename... Ts>
    void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node, std::pair<const char*, Ts&>... objects)
    {
		using expand_type = int[];
		// TODO: construct vector of all child nodes of xml_node to map to each object
		expand_type{ (DeserializeHumanReadableIfNecessary(xml_node, objects.first, objects.second), 0)... };
    }

	template<typename T>
	void DeserializeHumanReadable(std::istream& xml_istream, T& root_object)
	{
		xml_doc_.parse<>(xml_istream.rdbuf());

		dynamic_memory_node_ = xml_doc_.last_node();
		for () {
			// store dymanic memory objects
		}

		DeserializeHumanReadableIfNecessary(*xml_doc_.first_node(), root_object);

		rapidxml::xml_node<>* dynamic_memory_node = xml_doc_.last_node();
		

		id_to_object_map_.clear();
		object_to_id_map_.clear();
	}
	*/

private:
	std::uint32_t next_id_;
    std::unordered_map<void*, ArchiveNodeBase*> object_to_node_map_;
	std::unordered_map<void*, ArchiveNodeBase*> dynamic_memory_candidate_nodes_map_;

	std::size_t IdForObject(void* object_ptr)
	{
		if (object_to_node_map_.find(object_ptr) != object_to_node_map_.end()) {
			return object_to_node_map_[object_ptr]->Id();
		}
		return 0;
	}

	void SerializeHumanReadableFromNode(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
	{
		std::unordered_map<std::uint32_t, rapidxml::xml_node<>*> child_id_to_parent_xml_node;
		std::stack<ArchiveNodeBase*> dfs_stack;
		child_id_to_parent_xml_node[root_node->Id()] = root_xml_node;
		dfs_stack.push(root_node);
		while (!dfs_stack.empty()) {
			ArchiveNodeBase* node = dfs_stack.top();
			dfs_stack.pop();

			// Serialize node.
			rapidxml::xml_node<>* xml_node = node->SerializeHumanReadable(xml_doc);

			// Add serialized node as child to parent.
			child_id_to_parent_xml_node[node->Id()]->append_node(xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveNodeBase*> children = node->GetChildArchiveNodes(*this);
			for (std::vector<ArchiveNodeBase*>::reverse_iterator it = children.rbegin(); it != children.rend(); ++it)
			{
				ArchiveNodeBase* child_node = *it;
				child_id_to_parent_xml_node[child_node->Id()] = xml_node;
				dfs_stack.push(child_node);
			}
		}
	}

	/*
	// Override for std::vector
	template<typename T>
	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node, std::vector<T>& obj_vec)
	{
		std::size_t vec_count;
		xml_node.first_node("count")->value() >> vec_count;

		obj_vec = std::vector<T>();
		obj_vec.reserve(vec_count);
		for (std::size_t i = 0; i < vec_count; ++i) {
			T object;
			DeserializeHumanReadableIfNecessary(xml_node, object);
			obj_vec.push_back(object);
		}
	}

	template<typename T>
	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node, T*& object)
	{
		std::size_t pointee_id = IdForObject((void*)object);
		if (!pointee_id) {
			xml_node.first_node("pointee")->first_attribute("pointee_id")->value() >> pointee_id;
			if (is_dynamic_memory) {
				T object_ptr;
				DeserializeHumanReadableIfNecessary(xml_node)
			}
			else {

			}
			// TODO: broadcast to all unresolved pointers encountered so far
		}
		else {
			*object = static_cast<T*>(id_to_object_map_[pointee_id]);
		}

		std::size_t pointee_id = IdForObject((void*)object);
		if (!pointee_id) {
			// Serialize object being pointed to. Does not handle the case of a pointer to an array of objects.
			// Initially, we assume the pointer points to a dynamically allocated object, and so we make the object node a child of the dynamic memory node.
			SerializeHumanReadableIfNecessary(*dynamic_memory_node_, "object", *object);
			pointee_id = IdForObject((void*)object);
		}

		rapidxml::xml_node<>* pointer_node = xml_doc_.allocate_node(rapidxml::node_element, "pointee");
		char* pointee_id_string = xml_doc_.allocate_string(std::to_string(pointee_id).c_str());
		rapidxml::xml_attribute<>* pointee_id_atttribute = xml_doc_.allocate_attribute("pointee_id", pointee_id_string);
		pointer_node->append_attribute(pointee_id_atttribute);
		xml_node.append_node(pointer_node);
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type
		DerializeHumanReadable(rapidxml::xml_node<>& xml_node, T& object)
	{
		object.DerializeHumanReadable(*this, xml_node);
	}

	template<typename T>
	typename std::enable_if<!std::is_base_of<ISerializable, T>::value>::type
		DerializeHumanReadable(rapidxml::xml_node<>& xml_node, T& object)
	{
		xml_node.value() >> object;
	}

	template<typename T>
	void DeserializeHumanReadableIfNecessary(rapidxml::xml_node<>& xml_node, T& object)
	{
		std::size_t id;
		xml_node.first_attribute("id")->value() >> id;
		id_to_object_map_[id] = &object;
		id_to_node_map_[id] = &xml_node;
		DerializeHumanReadable(xml_node, object);
	}
	*/
};