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

    Archive() 
	{
		next_id_ = 0;
	}

	template<typename T>
	ArchiveNodeBase* RegisterObject(const char* name, T& object)
	{
		// Important to do ++next_id_ because we want 0 to be the null id.
		std::uint32_t object_id = ++next_id_;

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_to_pointer_node_map_.find(&object);
		if (iter != pointee_to_pointer_node_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_pointer_node_map_.erase((void*)&object);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object, object_id);
			}
		}

		ArchiveNodeBase* object_node = (ArchiveNodeBase*) new ArchiveObjectNode<T>(object_id, name, object);
		object_to_node_map_[(void*)&object] = object_node;
		return object_node;
	}

	template<typename T>
	ArchiveNodeBase* RegisterObject(const char* name, T*& object_ptr)
	{
		// Important to do ++next_id_ because we want 0 to be the null id.
		const std::uint32_t pointer_id = ++next_id_;

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator iter = pointee_to_pointer_node_map_.find(&object_ptr);
		if (iter != pointee_to_pointer_node_map_.end()) {
			const std::vector<ArchivePointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_pointer_node_map_.erase((void*)&object_ptr);
			for (ArchivePointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(xml_doc_, &object_ptr, pointer_id);
			}
		}

		const std::uint32_t pointee_id = IdForObject((void*)object_ptr);
		ArchivePointerNodeBase* pointer_node = (ArchivePointerNodeBase*) new ArchivePointerNode<T>(pointer_id, name, pointee_id, object_ptr);
		if (!pointee_id) {
			// The object pointed to by object_ptr has not been registered yet.
			if (pointee_to_pointer_node_map_.find((void*)object_ptr) != pointee_to_pointer_node_map_.end()) {
				pointee_to_pointer_node_map_[(void*)object_ptr].push_back(pointer_node);
			}
			else {
				pointee_to_pointer_node_map_[(void*)object_ptr] = { pointer_node };
			}
		}

		object_to_node_map_[(void*)&object_ptr] = (ArchiveNodeBase*)pointer_node;
		return (ArchiveNodeBase*)pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveNodeBase*> RegisterObjects(std::pair<const char*, Ts&>... objects)
	{
		const std::vector<ArchiveNodeBase *> child_nodes = { (RegisterObject(objects.first, objects.second))... };
		return child_nodes;
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_.allocate_node(rapidxml::node_element, "dynamic_memory");
		next_id_ = 0;

		SerializeHumanReadableFromNodeBFS(xml_doc_, &xml_doc_, RegisterObject(name, root_object));

		while (!pointee_to_pointer_node_map_.empty()) {
			std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator first_node_pair_iter = pointee_to_pointer_node_map_.begin();
			void* object_addr = first_node_pair_iter->first;
			ArchivePointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			ArchiveNodeBase* pointee_node = dynamic_memory_pointer_node->PointeeNode(*this);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_to_pointer_node_map_.find(object_addr) == pointee_to_pointer_node_map_.end());
			SerializeHumanReadableFromNodeBFS(xml_doc_, dynamic_memory_xml_node, pointee_node);
		}

		xml_doc_.append_node(dynamic_memory_xml_node);
		xml_ostream << xml_doc_;

		for (std::pair<void*, ArchiveNodeBase*> object_node_pair : object_to_node_map_) {
			delete object_node_pair.second;
		}
		object_to_node_map_.clear();
		xml_doc_.clear();
	}

	template<typename T>
	void DeserializeHumanReadable(std::istream& xml_istream, const char* name, T& root_object)
	{
		std::vector<char> buffer((std::istreambuf_iterator<char>(xml_istream)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
		xml_doc_.parse<0>(buffer.data());

		next_id_ = 0;
		std::vector<ArchiveNodeBase*> bfs_nodes = DeserializeHumanReadableFromNodeBFS(xml_doc_, xml_doc_.first_node(), RegisterObject(name, root_object));

		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_.allocate_node(rapidxml::node_element, "dynamic_memory");
		rapidxml::xml_node<>* dynamic_memory_child_xml_node = dynamic_memory_xml_node->first_node();
		while (!pointee_to_pointer_node_map_.empty()) {
			std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>>::iterator first_node_pair_iter = pointee_to_pointer_node_map_.begin();
			void* object_addr = first_node_pair_iter->first;
			ArchivePointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			dynamic_memory_pointer_node->DynamicallyAllocateObject();
			ArchiveNodeBase* pointee_node = dynamic_memory_pointer_node->PointeeNode(*this);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_to_pointer_node_map_.find(object_addr) == pointee_to_pointer_node_map_.end());
			std::vector<ArchiveNodeBase*> dynamicallly_allocated_bfs_nodes = DeserializeHumanReadableFromNodeBFS(xml_doc_, dynamic_memory_child_xml_node, pointee_node);
			bfs_nodes.insert(bfs_nodes.end(), dynamicallly_allocated_bfs_nodes.begin(), dynamicallly_allocated_bfs_nodes.end());
			dynamic_memory_child_xml_node = dynamic_memory_child_xml_node->next_sibling();
		}

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveNodeBase*>::reverse_iterator it = bfs_nodes.rbegin(); it != bfs_nodes.rend(); ++it)
		{
			(*it)->ConstructFromDeserializedDependencies();
		}

		for (std::pair<void*, ArchiveNodeBase*> object_node_pair : object_to_node_map_) {
			delete object_node_pair.second;
		}

		object_to_node_map_.clear();
		xml_doc_.clear();
	}

private:
	std::uint32_t next_id_;
    std::unordered_map<void*, ArchiveNodeBase*> object_to_node_map_;
	std::unordered_map<void*, std::vector<ArchivePointerNodeBase*>> pointee_to_pointer_node_map_;
	rapidxml::xml_document<> xml_doc_;

	std::uint32_t IdForObject(void* object_ptr)
	{
		if (object_to_node_map_.find(object_ptr) != object_to_node_map_.end()) {
			return object_to_node_map_[object_ptr]->Id();
		}
		return 0;
	}

	void SerializeHumanReadableFromNodeDFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
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

	void SerializeHumanReadableFromNodeBFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
	{
		std::unordered_map<std::uint32_t, rapidxml::xml_node<>*> child_id_to_parent_xml_node;
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
			std::vector<ArchiveNodeBase*> children = node->GetChildArchiveNodes(*this);
			for (std::vector<ArchiveNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveNodeBase* child_node = *it;
				child_id_to_parent_xml_node[child_node->Id()] = xml_node;
				bfs_queue.push(child_node);
			}
		}
	}
	
	std::vector<ArchiveNodeBase*> DeserializeHumanReadableFromNodeBFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveNodeBase* root_node)
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
			std::cout << node->Name() << std::endl;
			node->DeserializeHumanReadable(*xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveNodeBase*> children = node->GetChildArchiveNodes(*this);
			rapidxml::xml_node<>* xml_child_node = xml_node->first_node();
			for (std::vector<ArchiveNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveNodeBase* child_node = *it;
				bfs_queue.push(child_node);
				bfs_xml_node_queue.push(xml_child_node);
				xml_child_node = xml_child_node->next_sibling();
			}
		}

		return bfs_nodes;
	}
	
};