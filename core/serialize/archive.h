#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <stack>
#include <queue>
#include <set>

#include "archive_ser_node.h"
#include "archive_des_node.h"

class Archive 
{
public:

    // TODO create map from strings to functions that will be used in case pointers cannot be figured out. This allows
    // serializing/deserializing function pointers

	// TODO insert version numbers at beginning of xml

    Archive() {
		xml_doc_ = new rapidxml::xml_document<>();
	}

	~Archive() {
		delete xml_doc_;
	}

	template<typename T>
	ArchiveSerNodeBase* RegisterObjectForSerialization(const char* name, T& object)
	{
		std::size_t object_id = StoreObject(&object);
		ArchiveSerNodeBase* object_node = (ArchiveSerNodeBase*) new ArchiveSerObjectNode<T>(object_id, name, object);
		ser_nodes_.push_back((ArchiveSerNodeBase*)object_node);

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchiveSerPointerNodeBase*>>::iterator iter = pointee_to_ser_pointer_nodes_map_.find(&object);
		if (iter != pointee_to_ser_pointer_nodes_map_.end()) {
			const std::vector<ArchiveSerPointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_ser_pointer_nodes_map_.erase((void*)&object);
			for (ArchiveSerPointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(*xml_doc_, &object, object_id);
			}
		}

		return object_node;
	}

	template<typename T>
	ArchiveSerNodeBase* RegisterObjectForSerialization(const char* name, T*& object_ptr)
	{
		const std::size_t pointer_id = StoreObject(&object_ptr);
		const std::size_t pointee_id = IdForObject((void*)object_ptr);
		ArchiveSerPointerNodeBase* pointer_node = (ArchiveSerPointerNodeBase*) new ArchiveSerPointerNode<T>(pointer_id, name, pointee_id, object_ptr);
		ser_nodes_.push_back((ArchiveSerNodeBase*)pointer_node);
		if (!pointee_id) {
			// The object pointed to by object_ptr has not been registered yet.
			if (pointee_to_ser_pointer_nodes_map_.find((void*)object_ptr) != pointee_to_ser_pointer_nodes_map_.end()) {
				pointee_to_ser_pointer_nodes_map_[(void*)object_ptr].push_back(pointer_node);
			}
			else {
				pointee_to_ser_pointer_nodes_map_[(void*)object_ptr] = { pointer_node };
			}
		}

		// Notify listeners for this object's address
		std::unordered_map<void*, std::vector<ArchiveSerPointerNodeBase*>>::iterator iter = pointee_to_ser_pointer_nodes_map_.find(&object_ptr);
		if (iter != pointee_to_ser_pointer_nodes_map_.end()) {
			const std::vector<ArchiveSerPointerNodeBase*> pointer_listeners = iter->second;
			pointee_to_ser_pointer_nodes_map_.erase((void*)&object_ptr);
			for (ArchiveSerPointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(*xml_doc_, &object_ptr, pointer_id);
			}
		}

		return (ArchiveSerNodeBase*)pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveSerNodeBase*> RegisterObjectsForSerialization(std::pair<const char*, Ts&>... objects)
	{
		const std::vector<ArchiveSerNodeBase*> child_nodes = { (RegisterObjectForSerialization(objects.first, objects.second))... };
		return child_nodes;
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_->allocate_node(rapidxml::node_element, "dynamic_memory");

		ser_nodes_ = { nullptr };
		objects_ = { nullptr };
		SerializeHumanReadableFromNodeBFS(*xml_doc_, xml_doc_, RegisterObjectForSerialization(name, root_object));

		while (!pointee_to_ser_pointer_nodes_map_.empty()) {
			std::unordered_map<void*, std::vector<ArchiveSerPointerNodeBase*>>::iterator first_node_pair_iter = pointee_to_ser_pointer_nodes_map_.begin();
			void* object_addr = first_node_pair_iter->first;
			ArchiveSerPointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			ArchiveSerNodeBase* pointee_node = dynamic_memory_pointer_node->PointeeNode(*this);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_to_ser_pointer_nodes_map_.find(object_addr) == pointee_to_ser_pointer_nodes_map_.end());
			SerializeHumanReadableFromNodeBFS(*xml_doc_, dynamic_memory_xml_node, pointee_node);
		}

		xml_doc_->append_node(dynamic_memory_xml_node);
		xml_ostream << *xml_doc_;

		for (auto it = std::next(ser_nodes_.begin()); it != ser_nodes_.end(); ++it) {
			delete *it;
		}

		ser_nodes_.clear();
		xml_doc_->clear();
	}

	template<typename T>
	ArchiveDesNodeBase* RegisterObjectForDeserialization(rapidxml::xml_node<>& xml_node, T& object)
	{
		const char* name = xml_node.name();
		const std::size_t object_id = StoreObject(&object);
		assert(IdForXMLNode(xml_node) == object_id);

		ArchiveDesNodeBase* object_node = (ArchiveDesNodeBase*) new ArchiveDesObjectNode<T>(object_id, name, object);
		des_nodes_.push_back((ArchiveDesNodeBase*)object_node);
		
		// Notify listeners for this object's id
		std::unordered_map<std::size_t, std::vector<ArchiveDesPointerNodeBase*>>::iterator iter = pointee_id_to_des_pointer_nodes_map_.find(object_id);
		if (iter != pointee_id_to_des_pointer_nodes_map_.end()) {
			const std::vector<ArchiveDesPointerNodeBase*> pointer_listeners = iter->second;
			pointee_id_to_des_pointer_nodes_map_.erase(object_id);
			for (ArchiveDesPointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(&object, object_id);
			}
		}

		return object_node;
	}

	template<typename T>
	ArchiveDesNodeBase* RegisterObjectForDeserialization(rapidxml::xml_node<>& xml_node, T*& object_ptr)
	{
		const char* name = xml_node.name();
		const std::size_t pointer_id = StoreObject(&object_ptr);
		assert(IdForXMLNode(xml_node) == pointer_id);

		const std::size_t pointee_id = PointeeIdForXMLNode(xml_node);
		ArchiveDesPointerNodeBase* pointer_node = (ArchiveDesPointerNodeBase*) new ArchiveDesPointerNode<T>(pointer_id, name, pointee_id, object_ptr);
		des_nodes_.push_back((ArchiveDesNodeBase*)pointer_node);

		void* pointee_object = ObjectForId(pointee_id);
		if (pointee_object) {
			pointer_node->DidRegisterPointee(objects_[pointee_id], pointee_id);
		}
		else {
			// The object pointed to by object_ptr has not been registered yet.
			if (pointee_id_to_des_pointer_nodes_map_.find(pointee_id) != pointee_id_to_des_pointer_nodes_map_.end()) {
				pointee_id_to_des_pointer_nodes_map_[pointee_id].push_back(pointer_node);
			}
			else {
				pointee_id_to_des_pointer_nodes_map_[pointee_id] = { pointer_node };
			}
		}

		// Notify listeners for this object's id
		std::unordered_map<std::size_t, std::vector<ArchiveDesPointerNodeBase*>>::iterator iter = pointee_id_to_des_pointer_nodes_map_.find(pointer_id);
		if (iter != pointee_id_to_des_pointer_nodes_map_.end()) {
			const std::vector<ArchiveDesPointerNodeBase*> pointer_listeners = iter->second;
			pointee_id_to_des_pointer_nodes_map_.erase(pointer_id);
			for (ArchiveDesPointerNodeBase* pointer_listener : pointer_listeners) {
				pointer_listener->DidRegisterPointee(&object_ptr, pointer_id);
			}
		}

		return (ArchiveDesNodeBase*)pointer_node;
	}

	template<typename... Ts>
	std::vector<ArchiveDesNodeBase*> RegisterObjectsForDeserialization(rapidxml::xml_node<>& parent_xml_node, Ts&... objects)
	{
		std::vector<rapidxml::xml_node<>*> children_xml_nodes;
		rapidxml::xml_node<>* child_node = parent_xml_node.first_node();
		while (child_node) {
			children_xml_nodes.push_back(child_node);
			child_node = child_node->next_sibling();
		}
		
		int index = 0;
		const std::vector<ArchiveDesNodeBase*> child_nodes = { (RegisterObjectForDeserialization(*children_xml_nodes[index++], objects))... };
		return child_nodes;
	}

	template<typename T>
	void DeserializeHumanReadable(std::istream& xml_istream, T& root_object)
	{
		std::vector<char> buffer((std::istreambuf_iterator<char>(xml_istream)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
		xml_doc_->parse<0>(buffer.data());

		des_nodes_ = { nullptr };
		objects_ = { nullptr };
		DeserializeHumanReadableFromNodeBFS(xml_doc_->first_node(), RegisterObjectForDeserialization(*xml_doc_->first_node(), root_object));
		rapidxml::xml_node<>* dynamic_memory_xml_node = xml_doc_->last_node("dynamic_memory");
		rapidxml::xml_node<>* dynamic_memory_child_xml_node = dynamic_memory_xml_node->first_node();
		while (!pointee_id_to_des_pointer_nodes_map_.empty()) {
			std::unordered_map<std::size_t, std::vector<ArchiveDesPointerNodeBase*>>::iterator first_node_pair_iter = pointee_id_to_des_pointer_nodes_map_.begin();
			const std::size_t object_id = first_node_pair_iter->first;
			ArchiveDesPointerNodeBase* dynamic_memory_pointer_node = first_node_pair_iter->second.front();
			dynamic_memory_pointer_node->DynamicallyAllocateObject(*dynamic_memory_child_xml_node);
			ArchiveDesNodeBase* pointee_node = dynamic_memory_pointer_node->PointeeNode(*this, *dynamic_memory_child_xml_node);
			// Calling PointeeNode should have removed this dynamically-allocated node from pointee_to_pointer_node_map_.
			assert(pointee_id_to_des_pointer_nodes_map_.find(object_id) == pointee_id_to_des_pointer_nodes_map_.end());
			DeserializeHumanReadableFromNodeBFS(dynamic_memory_child_xml_node, pointee_node);
			dynamic_memory_child_xml_node = dynamic_memory_child_xml_node->next_sibling();
		}

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveDesNodeBase*>::reverse_iterator it = des_nodes_.rbegin(); it != std::prev(des_nodes_.rend()); ++it) {
			ArchiveDesNodeBase* node = *it;
			node->ConstructFromDeserializedDependencies();
			delete node;
		}

		des_nodes_.clear();
		objects_.clear();
		xml_doc_->clear();
	}

	// Assumes no dynamically allocated objects within root_object.
	template<typename T>
	void DeserializeHumanReadable(rapidxml::xml_node<>& root_node, T& root_object)
	{
		pointee_id_to_des_pointer_nodes_map_ = {};
		des_nodes_ = { nullptr };
		objects_ = { nullptr };

		DeserializeHumanReadableFromNodeBFS(&root_node, RegisterObjectForDeserialization(root_node, root_object));

		// We must do this in reverse to ensure that dependencies are constructed before their parents are.
		for (std::vector<ArchiveDesNodeBase*>::reverse_iterator it = des_nodes_.rbegin(); it != std::prev(des_nodes_.rend()); ++it) {
			ArchiveDesNodeBase* node = *it;
			node->ConstructFromDeserializedDependencies();
			delete node;
		}

		pointee_id_to_des_pointer_nodes_map_.clear();
		des_nodes_.clear();
		objects_.clear();
	}

private:
	std::unordered_map<void*, std::size_t> object_to_id_map_;
	std::vector<void*> objects_;
	// BFS order
	std::vector<ArchiveSerNodeBase*> ser_nodes_;
	std::vector<ArchiveDesNodeBase*> des_nodes_;
	std::unordered_map<void*, std::vector<ArchiveSerPointerNodeBase*>> pointee_to_ser_pointer_nodes_map_;
	std::unordered_map<std::size_t, std::vector<ArchiveDesPointerNodeBase*>> pointee_id_to_des_pointer_nodes_map_;
	rapidxml::xml_document<>* xml_doc_;

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

	void SerializeHumanReadableFromNodeBFS(rapidxml::xml_document<>& xml_doc, rapidxml::xml_node<>* root_xml_node, ArchiveSerNodeBase* root_node)
	{
		std::unordered_map<std::size_t, rapidxml::xml_node<>*> child_id_to_parent_xml_node;
		std::queue<ArchiveSerNodeBase*> bfs_queue;
		child_id_to_parent_xml_node[root_node->Id()] = root_xml_node;
		bfs_queue.push(root_node);
		while (!bfs_queue.empty()) {
			ArchiveSerNodeBase* node = bfs_queue.front();
			bfs_queue.pop();

			// Serialize node.
			rapidxml::xml_node<>* xml_node = node->SerializeHumanReadable(xml_doc);

			// Add serialized node as child to parent.
			child_id_to_parent_xml_node[node->Id()]->append_node(xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveSerNodeBase*> children = node->GetChildArchiveNodes(*this);
			for (std::vector<ArchiveSerNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveSerNodeBase* child_node = *it;
				child_id_to_parent_xml_node[child_node->Id()] = xml_node;
				bfs_queue.push(child_node);
			}
		}
	}
	
	void DeserializeHumanReadableFromNodeBFS(rapidxml::xml_node<>* root_xml_node, ArchiveDesNodeBase* root_node)
	{
		std::vector<ArchiveDesNodeBase*> bfs_nodes;
		std::queue<ArchiveDesNodeBase*> bfs_queue;
		bfs_queue.push(root_node);
		std::queue<rapidxml::xml_node<>*> bfs_xml_node_queue;
		bfs_xml_node_queue.push(root_xml_node);
		while (!bfs_queue.empty()) {
			ArchiveDesNodeBase* node = bfs_queue.front();
			bfs_nodes.push_back(node);
			bfs_queue.pop();

			rapidxml::xml_node<>* xml_node = bfs_xml_node_queue.front();
			bfs_xml_node_queue.pop();

			// Deserialize node.
			node->DeserializeHumanReadable(*xml_node);

			// Iterate children in reverse.
			std::vector<ArchiveDesNodeBase*> children = node->GetChildArchiveNodes(*this, *xml_node);
			rapidxml::xml_node<>* child_xml_node = xml_node->first_node();
			for (std::vector<ArchiveDesNodeBase*>::iterator it = children.begin(); it != children.end(); ++it)
			{
				ArchiveDesNodeBase* child_node = *it;
				bfs_queue.push(child_node);
				bfs_xml_node_queue.push(child_xml_node);
				child_xml_node = child_xml_node->next_sibling();
			}
		}
	}
	
};