#pragma once

#include <iostream>
#include <unordered_map>

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
    void SerializeHumanReadable(std::ostream& xml_ostream, std::pair<std::string, Ts&> ...objects)
    {
		{ SerializeHumanReadableIfAble(xml_ostream, objects.first, objects.second)... };
    }

	template<typename T>
	std::ostream& SerializeHumanReadable(std::ostream& xml_ostream, T& root_object)
	{
		root_object.Serialize(this, xml_ostream);

		// It is assumed that any unresolved pointers point to objects allocated on the heap.
		xml_ostream << "<Heap>\n";
		for (std::pair<std::size_t, std::ostream> pair : unresolved_pointer_serializations_) {
			xml_ostream << pair.second;
		}
		xml_ostream << "<\Heap>\n";
		unresolved_pointer_serializations_.clear();
		id_to_object_map_.clear();
		object_to_id_map_.clear();
		return xml_ostream;
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
	std::unordered_map<std::size_t, std::ostream> unresolved_pointer_serializations_;

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

	// Override for std::shared_ptrs
	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, std::shared_ptr<T>& obj_shared_ptr)
	{
		xml_ostream << "<" << name << " type=SharedPointer id=" + id + ">\n";
		SerializeHumanReadableIfAble(xml_ostream, "ptr", obj_shared_ptr.get());
		xml_ostream << "</" + name + ">\n";
	}

	// Override for std::vectors
	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, std::vector<T>& obj_vec)
	{
		xml_ostream << "<" << name << " type=Vector id=" + id + ">\n";
		xml_ostream << "<count>" << obj_vec.size() << "</count>\n";
		for (T& object : obj_vec) {
			SerializeHumanReadableIfAble(xml_ostream, "element", object);
		}
		xml_ostream << "</" + name + ">\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, T*& object)
	{
		std::size_t pointee_id = IdForObject((void*)object);
		if (!pointee_id) {
			pointee_id = Store((void*)object);
			// Serialize object being pointed to and . Does not handle the case of a pointer to an array of objects.
			std::ostream temp_ostream;
			SerializeHumanReadableIfAble(temp_ostream, std::empty, *object);
			unresolved_pointer_serializations_.insert({ pointee_id , temp_ostream });
		}
		xml_ostream << "<Pointer pointee_id=" << pointee_id << "/>\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, T& object)
	{
		ISerializable* object_serializable = dynamic_cast<ISerializable*>(object);
		if (serializable) {
			object_serializable->SerializeHumanReadable(this, xml_ostream);
		}
		else {
			xml_ostream << object;
		}
	}

	template<typename T>
	void SerializeHumanReadableIfAble(std::ostream& xml_ostream, std::string name, T& object)
	{
		std::size_t id = IdForObject((void*)&object);
		if (!id) {
			id = Store((void*)&object);
			if (name != std::empty) {
				xml_ostream << "<" << name << " id=" << id << ">\n";
			}
			SerializeHumanReadable(xml_ostream, name, object);
		}
		else {
			std::unordered_map<std::size_t, std::ostream>::iterator iter = unresolved_pointer_serializations_.find(id);
			if (iter == unresolved_pointer_serializations_.end()) {
				return;
			}

			// We have already visited and serialized this object. This is a cycle. Stop recursing.
			if (name != std::empty) {
				xml_ostream << "<" << name << " id=" << id << ">\n";
			}
			xml_ostream << iter->second;
			unresolved_pointer_serializations_.erase(id);
			return;
		}

		if (name != std::empty) {
			xml_ostream << "</" << name << ">\n";
		}
	}
};