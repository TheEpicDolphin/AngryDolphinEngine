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
		id_to_node_map_.push_back(nullptr);
    }

    template<typename... Ts>
    void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, std::pair<const char *, Ts&>... objects)
    {
		using expand_type = int[];
		expand_type{ (SerializeHumanReadableIfNecessary(xml_node, objects.first, objects.second), 0)... };
    }

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, const char* name, T& root_object)
	{
		dynamic_memory_node_ = xml_doc_.allocate_node(rapidxml::node_element, "dynamic_memory");
		SerializeHumanReadableIfNecessary(xml_doc_, name, root_object);
		xml_doc_.append_node(dynamic_memory_node_);

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
	std::vector<rapidxml::xml_node<>*> id_to_node_map_;
    std::unordered_map<void*, std::size_t> object_to_id_map_;
	rapidxml::xml_document<> xml_doc_;
	rapidxml::xml_node<> *dynamic_memory_node_;

	std::size_t Store(void* object_ptr, rapidxml::xml_node<> *node)
	{
		const std::size_t id = id_to_object_map_.size();
		id_to_object_map_.push_back(object_ptr);
		id_to_node_map_.push_back(node);
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
		SerializeHumanReadableIfNecessary(xml_node, "ptr", obj_shared_ptr.get());
	}

	// Override for std::vector
	template<typename T>
	void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, std::vector<T>& obj_vec)
	{
		char* vector_count_string = xml_doc_.allocate_string(std::to_string(obj_vec.size()).c_str());
		rapidxml::xml_node<>* count_node = xml_doc_.allocate_node(rapidxml::node_element, "count", vector_count_string);
		xml_node.append_node(count_node);

		for (T& object : obj_vec) {
			SerializeHumanReadableIfNecessary(xml_node, "element", object);
		}
	}
	
	template<typename T>
	void SerializeHumanReadable(rapidxml::xml_node<>& xml_node, T*& object)
	{
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
	void SerializeHumanReadableIfNecessary(rapidxml::xml_node<>& parent_xml_node, const char* name, T& object)
	{
		std::size_t id = IdForObject((void*)&object);
		if (!id) {
			rapidxml::xml_node<>* object_node = xml_doc_.allocate_node(rapidxml::node_element, name);
			id = Store((void*)&object, object_node);
			char* object_id_string = xml_doc_.allocate_string(std::to_string(id).c_str());
			rapidxml::xml_attribute<>* object_id_atttribute = xml_doc_.allocate_attribute("id", object_id_string);
			object_node->append_attribute(object_id_atttribute);
			SerializeHumanReadable(*object_node, object);
			parent_xml_node.append_node(object_node);
		}
		else {
			rapidxml::xml_node<> *serialized_node = id_to_node_map_[id];
			if (serialized_node->parent() == dynamic_memory_node_) {
				return;
			}

			// This object was not dynamically allocated.
			dynamic_memory_node_->remove_node(serialized_node);
			// Update name of node
			serialized_node->name(name);
			// Add this object's serialization to the xml tree
			parent_xml_node.append_node(serialized_node);

			// We return here to break cycle.
			return;
		}
	}
};