#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

//#include <rapidxml/rapidxml.hpp>
//#include <rapidxml/rapidxml_print.hpp>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "serializable.h"

class Archive 
{
public:

    // TODO create map from strings to functions that will be used in case pointers cannot be figured out. This allows
    // serializing/deserializing function pointers

	// TODO insert version numbers at beginning of xml

    Archive() {
        id_to_object_map_.push_back(nullptr);
    }

    template<typename... Ts>
    void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, std::pair<const char *, Ts&>... objects)
    {
		using expand_type = int[];
		expand_type{ (SerializeHumanReadableIfAble(xml_node, objects.first, objects.second), 0)... };
    }

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		SerializeHumanReadableIfAble(xml_doc_, name, root_object);

		// It is assumed that any unresolved pointers point to objects allocated on the heap.
		rapidxml::xml_node<> *heap_node = xml_doc_.allocate_node(rapidxml::node_element, "heap");
		for (auto& serializations_iter : unresolved_pointer_serializations_) {
			rapidxml::xml_node<>* object_node = xml_doc_.allocate_node(rapidxml::node_element, "object");
			rapidxml::xml_attribute<>* id_attribute = xml_doc_.allocate_attribute("id", std::to_string(serializations_iter.first).c_str());
			object_node->append_attribute(id_attribute);
			rapidxml::xml_node<>* serialized_node = serializations_iter.second;
			object_node->append_node(serialized_node);
			heap_node->append_node(object_node);
		}
		xml_doc_.append_node(heap_node);
		
		/*
		xml_ostream << "<Heap>\n";
		for (auto& serializations_iter : unresolved_pointer_serializations_) {
			xml_ostream << "<Object" << " id=" << serializations_iter.first << ">\n";
			xml_ostream << serializations_iter.second.rdbuf();
			xml_ostream << "<Object/>\n";
		}
		xml_ostream << "</Heap>\n";
		*/

		unresolved_pointer_serializations_.clear();
		id_to_object_map_.clear();
		object_to_id_map_.clear();

		xml_ostream << xml_doc_;
	}

	// completion is called when all the args have been retrieved from the XML file.
	template<typename T, typename... Args>
	void DeserializeHumanReadableWithCompletion(T object, std::pair<std::string, Args&> ...args, void (*completion)(T&, Args&...))
	{

	}

    template<typename... Ts>
    void DeserializeHumanReadable(std::pair<std::string, Ts&> ...objects)
    {

    }

private:
    std::vector<void*> id_to_object_map_;
    std::unordered_map<void*, std::size_t> object_to_id_map_;
	std::unordered_map<std::size_t, rapidxml::xml_node<> *> unresolved_pointer_serializations_;
	rapidxml::xml_document<> xml_doc_;

	/*
	template<typename T>
	static const char* ValueToCharArray(T& value) 
	{
		std::string tmp = std::to_string(value);
		return tmp.c_str();
	}
	*/

	std::size_t Store(void* object_ptr)
	{
		const std::size_t id = id_to_object_map_.size();
		id_to_object_map_.push_back(object_ptr);
		object_to_id_map_[object_ptr] = id;
		return id;
	}

	std::size_t IdForObject(void* object_ptr)
	{
		if (object_to_id_map_.find(object_ptr) != object_to_id_map_.end()) {
			return object_to_id_map_[object_ptr];
		}
		return 0;
	}

	// Override for std::shared_ptr
	template<typename T>
	void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, std::shared_ptr<T>& obj_shared_ptr)
	{
		SerializeHumanReadableIfAble(xml_node, "ptr", obj_shared_ptr.get());
	}

	// Override for std::vector
	template<typename T>
	void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, std::vector<T>& obj_vec)
	{
		char* vector_count_string = xml_doc_.allocate_string(std::to_string(obj_vec.size()).c_str());
		rapidxml::xml_node<>* count_node = xml_doc_.allocate_node(rapidxml::node_element, "count", vector_count_string);
		xml_node.append_node(count_node);

		for (T& object : obj_vec) {
			SerializeHumanReadableIfAble(xml_node, "element", object);
		}
	}
	
	template<typename T>
	void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, T*& object)
	{
		std::size_t pointee_id = IdForObject((void*)object);
		if (!pointee_id) {
			pointee_id = Store((void*)object);
			// Serialize object being pointed to. Does not handle the case of a pointer to an array of objects.
			rapidxml::xml_node<>* dummy_root_node = xml_doc_.allocate_node(rapidxml::node_element, "dummy_root");
			SerializeHumanReadableIfAble(*dummy_root_node, "temp_name", *object);
			unresolved_pointer_serializations_.insert({ pointee_id , dummy_root_node->first_node() });
		}

		rapidxml::xml_node<>* pointer_node = xml_doc_.allocate_node(rapidxml::node_element, "pointee");
		char* pointee_id_string = xml_doc_.allocate_string(std::to_string(pointee_id).c_str());
		rapidxml::xml_attribute<>* pointee_id_atttribute = xml_doc_.allocate_attribute("pointee_id", pointee_id_string);
		pointer_node->append_attribute(pointee_id_atttribute);
		xml_node.append_node(pointer_node);
	}

	template<typename T>
	typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type
		SerializeHumanReadable(rapidxml::xml_node<>& xml_node, T& object)
	{
		object.SerializeHumanReadable(*this, xml_node);
	}

	template<typename T>
	typename std::enable_if<!std::is_base_of<ISerializable, T>::value>::type
		SerializeHumanReadable(rapidxml::xml_node<>& xml_node, T& object)
	{
		std::stringstream tmp_ss;
		tmp_ss << object;
		xml_node.value(xml_doc_.allocate_string(tmp_ss.str().c_str()));
	}

	template<typename T>
	void SerializeHumanReadableIfAble(rapidxml::xml_node<>& parent_xml_node, const char* name, T& object)
	{
		std::size_t id = IdForObject((void*)&object);
		if (!id) {
			id = Store((void*)&object);

			rapidxml::xml_node<>* object_node = xml_doc_.allocate_node(rapidxml::node_element, name);
			char* object_id_string = xml_doc_.allocate_string(std::to_string(id).c_str());
			rapidxml::xml_attribute<>* object_id_atttribute = xml_doc_.allocate_attribute("id", object_id_string);
			object_node->append_attribute(object_id_atttribute);
			SerializeHumanReadable(*object_node, object);
			parent_xml_node.append_node(object_node);
		}
		else {
			std::unordered_map<std::size_t, rapidxml::xml_node<> *>::iterator iter = unresolved_pointer_serializations_.find(id);
			if (iter == unresolved_pointer_serializations_.end()) {
				return;
			}

			// We have already visited and serialized this object. This is a cycle. 
			// Stop recursing and add this object's serialization to the xml tree

			// Update name of node because we didn't know it before
			iter->second->name(name);
			parent_xml_node.append_node(iter->second);
			unresolved_pointer_serializations_.erase(id);
			return;
		}
	}
};