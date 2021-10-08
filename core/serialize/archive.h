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
	ArchiveNodeBase* RegisterObjectForSerialization(const char* name, T& object)
	{
		std::size_t object_id = nodes_.size();

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_to_pointer_nodes_map_.find(&object);
		if (iter != pointee_to_pointer_nodes_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_pointer_nodes_map_.erase((void*)&object);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object, object_id);
			}
		}

		ArchiveNodeBase* object_node = (ArchiveNodeBase*) new ArchiveObjectNode<T>(object_id, name, object);
		object_to_id_map_[&object] = object_id;
		nodes_.push_back((ArchiveNodeBase*)object_node);
		return object_node;
	}

	template<typename T>
	ArchiveNodeBase* RegisterObjectForSerialization(const char* name, T*& object_ptr)
	{
		const std::size_t pointer_id = nodes_.size();

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_to_pointer_nodes_map_.find(&object_ptr);
		if (iter != pointee_to_pointer_nodes_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_pointer_nodes_map_.erase((void*)&object_ptr);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object_ptr, pointer_id);
			}
		}

		const std::size_t pointee_id = IdForObject((void*)object_ptr);
		ArchivePointerNodeBase* pointer_node = (ArchivePointerNodeBase*) new ArchivePointerNode<T>(pointer_id, name, pointee_id, object_ptr);
		if (!pointee_id) {
			// The object pointed to by object_ptr has not been registered yet.
			if (pointee_to_pointer_nodes_map_.find((void*)object_ptr) != pointee_to_pointer_nodes_map_.end()) {
				pointee_to_pointer_nodes_map_[(void*)object_ptr].push_back(pointer_node);
			}
			else {
				pointee_to_pointer_nodes_map_[(void*)object_ptr] = { pointer_node };
			}
		}

		object_to_id_map_[&object_ptr] = pointer_id;
		nodes_.push_back((ArchiveNodeBase*)pointer_node);
		return (ArchiveNodeBase*)pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveNodeBase*> RegisterObjectsForSerialization(std::pair<const char*, Ts&>... objects)
	{
		const std::vector<ArchiveNodeBase *> child_nodes = { (RegisterObjectForSerialization(objects.first, objects.second))... };
		return child_nodes;
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_.allocate_node(rapidxml::node_element, "dynamic_memory");

		nodes_.push_back(nullptr);
		SerializeHumanReadableFromNodeBFS(xml_doc_, &xml_doc_, RegisterObjectForSerialization(name, root_object));

		while (!pointee_to_pointer_nodes_map_.empty()) {
			std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator first_node_pair_iter = pointee_to_pointer_nodes_map_.begin();
			void* object_addr = first_node_pair_iter->first;
			ArchivePointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			ArchiveNodeBase* pointee_node = dynamic_memory_pointer_node->SerializablePointeeNode(*this);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_to_pointer_nodes_map_.find(object_addr) == pointee_to_pointer_nodes_map_.end());
			SerializeHumanReadableFromNodeBFS(xml_doc_, dynamic_memory_xml_node, pointee_node);
		}

		xml_doc_.append_node(dynamic_memory_xml_node);
		xml_ostream << xml_doc_;

		for (auto it = std::next(nodes_.begin()); it != nodes_.end(); ++it) {
			delete *it;
		}

		nodes_.clear();
		xml_doc_.clear();
	}

	template<typename T>
	ArchiveNodeBase* RegisterObjectForDeserialization(rapidxml::xml_node<>& xml_node, T& object)
	{
		const char* name = xml_node.name();
		const std::size_t object_id = IdForXMLNode(xml_node);;

		// Notify listeners for this object's id
		std::unordered_map<std::size_t, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_id_to_pointer_nodes_map_.find(object_id);
		if (iter != pointee_id_to_pointer_nodes_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_id_to_pointer_nodes_map_.erase(object_id);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object, object_id);
			}
		}

		ArchiveNodeBase* object_node = (ArchiveNodeBase*) new ArchiveObjectNode<T>(object_id, name, object);
		std::cout << name << " " << nodes_.size() << " " << object_id << std::endl;
		assert(nodes_.size() == object_id);
		object_to_id_map_[&object] = object_id;
		nodes_.push_back((ArchiveNodeBase*)object_node);
		return object_node;
	}

	template<typename T>
	ArchiveNodeBase* RegisterObjectForDeserialization(rapidxml::xml_node<>& xml_node, T*& object_ptr)
	{
		const char* name = xml_node.name();
		const std::size_t pointer_id = IdForXMLNode(xml_node);
		const std::size_t pointee_id = PointeeIdForXMLNode(xml_node);

		// Notify listeners for this object's id
		std::unordered_map<std::size_t, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_id_to_pointer_nodes_map_.find(pointer_id);
		if (iter != pointee_id_to_pointer_nodes_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_id_to_pointer_nodes_map_.erase(pointer_id);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object_ptr, pointer_id);
			}
		}

		ArchivePointerNodeBase* pointer_node = (ArchivePointerNodeBase*) new ArchivePointerNode<T>(pointer_id, name, pointee_id, object_ptr);
		if (pointee_id > nodes_.size()) {
			// The object pointed to by object_ptr has not been registered yet.
			if (pointee_id_to_pointer_nodes_map_.find(pointee_id) != pointee_id_to_pointer_nodes_map_.end()) {
				pointee_id_to_pointer_nodes_map_[pointee_id].push_back(pointer_node);
			}
			else {
				pointee_id_to_pointer_nodes_map_[pointee_id] = { pointer_node };
			}
		}

		assert(nodes_.size() == pointer_id);
		object_to_id_map_[&object_ptr] = pointer_id;
		nodes_.push_back((ArchiveNodeBase*)pointer_node);
		return (ArchiveNodeBase*)pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveNodeBase*> RegisterObjectsForDeserialization(rapidxml::xml_node<>& parent_xml_node, Ts&... objects)
	{
		std::vector<rapidxml::xml_node<>*> children_xml_nodes;
		rapidxml::xml_node<>* child_node = parent_xml_node.first_node();
		while (child_node) {
			children_xml_nodes.push_back(child_node);
			child_node = child_node->next_sibling();
		}
		
		int index = 0;
		const std::vector<ArchiveNodeBase*> child_nodes = { (RegisterObjectForDeserialization(*children_xml_nodes[index++], objects))... };
		return child_nodes;
	}

	template<typename T>
	void DeserializeHumanReadable(std::istream& xml_istream, const char* name, T& root_object)
	{
		std::vector<char> buffer((std::istreambuf_iterator<char>(xml_istream)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
		xml_doc_.parse<0>(buffer.data());

		nodes_.push_back(nullptr);
		DeserializeHumanReadableFromNodeBFS(xml_doc_, xml_doc_.first_node(), RegisterObjectForDeserialization(*xml_doc_.first_node(), root_object));

		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_.allocate_node(rapidxml::node_element, "dynamic_memory");
		rapidxml::xml_node<>* dynamic_memory_child_xml_node = dynamic_memory_xml_node->first_node();
		while (!pointee_id_to_pointer_nodes_map_.empty()) {
			std::unordered_map<std::size_t, std::vector<ArchivePointerNodeBase*>>::iterator first_node_pair_iter = pointee_id_to_pointer_nodes_map_.begin();
			const std::size_t object_id = first_node_pair_iter->first;
			ArchivePointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			dynamic_memory_pointer_node->DynamicallyAllocateObject();
			ArchiveNodeBase* pointee_node = dynamic_memory_pointer_node->DeserializablePointeeNode(*this, *dynamic_memory_child_xml_node);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_id_to_pointer_nodes_map_.find(object_id) == pointee_id_to_pointer_nodes_map_.end());
			DeserializeHumanReadableFromNodeBFS(xml_doc_, dynamic_memory_child_xml_node, pointee_node);
			dynamic_memory_child_xml_node = dynamic_memory_child_xml_node->next_sibling();
		}

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveNodeBase*>::reverse_iterator it = nodes_.rbegin(); it != std::prev(nodes_.rend()); ++it) {
			ArchiveNodeBase* node = *it;
			node->ConstructFromDeserializedDependencies();
			delete node;
		}

		nodes_.clear();
		xml_doc_.clear();
	}

private:
	std::unordered_map<void*, std::size_t> object_to_id_map_;
	// BFS order
	std::vector<ArchiveNodeBase*> nodes_;
	std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>> pointee_to_pointer_nodes_map_;
	std::unordered_map<std::size_t, std::vector<ArchivePointerNodeBase*>> pointee_id_to_pointer_nodes_map_;
	rapidxml::xml_document<> xml_doc_;

	std::size_t IdForObject(void* object_ptr)
	{
		if (object_to_id_map_.find(object_ptr) != object_to_id_map_.end()) {
			return object_to_id_map_[object_ptr];
		}
		return 0;
	}

	std::size_t IdForXMLNode(rapidxml::xml_node<>& xml_node) 
	{
		std::size_t id;
		std::stringstream tmp_ss;
		tmp_ss << xml_node.first_attribute("id")->value();
		tmp_ss >> id;
		return id;
	}

	std::size_t PointeeIdForXMLNode(rapidxml::xml_node<>& xml_node)
	{
		std::size_t id;
		std::stringstream tmp_ss;
		tmp_ss << xml_node.first_node("pointee")->first_attribute("pointee_id")->value();
		tmp_ss >> id;
		return id;
	}

	void SerializeHumanReadableFromNodeBFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
	{
		std::unordered_map<std::size_t, rapidxml::xml_node<>*> child_id_to_parent_xml_node;
		std::queue<ArchiveNodeBase*> bfs_queue;
		child_id_to_parent_xml_node[root_node->Id()] = root_xml_node;
		bfs_queue.push(root_node);
		while (!bfs_queue.empty()) {
			ArchiveNodeBase* node = bfs_queue.front();
			bfs_queue.pop();

			// Serialize node.
			rapidxml::xml_node<>* xml_node = node->SerializeHumanReadable(xml_doc);

			// Add serialized node as child to parent.
			child_id_to_parent_xml_node[node->Id()]->append_node(xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveNodeBase*> children = node->GetChildArchiveNodesForSerialization(*this);
			for (std::vector<ArchiveNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveNodeBase* child_node = *it;
				child_id_to_parent_xml_node[child_node->Id()] = xml_node;
				bfs_queue.push(child_node);
			}
		}
	}
	
	void DeserializeHumanReadableFromNodeBFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
	{
		std::vector<ArchiveNodeBase*> bfs_nodes;
		std::queue<ArchiveNodeBase*> bfs_queue;
		bfs_queue.push(root_node);
		std::queue<rapidxml::xml_node<>*> bfs_xml_node_queue;
		bfs_xml_node_queue.push(root_xml_node);
		while (!bfs_queue.empty()) {
			ArchiveNodeBase* node = bfs_queue.front();
			bfs_nodes.push_back(node);
			bfs_queue.pop();

			rapidxml::xml_node<>* xml_node = bfs_xml_node_queue.front();
			bfs_xml_node_queue.pop();

			// Deserialize node.
			node->DeserializeHumanReadable(*xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveNodeBase*> children = node->GetChildArchiveNodesForDeserialization(*this, *xml_node);
			rapidxml::xml_node<>* xml_child_node = xml_node->first_node();
			for (std::vector<ArchiveNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveNodeBase* child_node = *it;
				bfs_queue.push(child_node);
				bfs_xml_node_queue.push(xml_child_node);
				xml_child_node = xml_child_node->next_sibling();
			}
		}
	}
	
};