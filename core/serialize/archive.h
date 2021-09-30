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
		{ SerializeHumanReadable(xml_ostream, objects.first, objects.second)... };
    }

	template<typename T>
	std::ostream& SerializeHumanReadable(std::ostream& xml_ostream, T& root_object)
	{
		root_object.Serialize(this, xml_ostream);

		// It is assumed that any unresolved pointers point to objects allocated on the heap.
		xml_ostream << "<Heap>\n";
		for (std::pair<std::size_t, std::ostream> pair : unresolved_pointer_ostreams_) {
			xml_ostream << pair.second;
		}
		xml_ostream << "<\Heap>\n";
		unresolved_pointer_ostreams_.clear();
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
	std::unordered_map<std::size_t, std::ostream> unresolved_pointer_ostreams_;

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

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, std::shared_ptr<T>& obj_shared_ptr)
	{
		std::size_t id = IdForObject((void*)&obj_shared_ptr);
		if (!id) {
			id = Store((void*)&obj_shared_ptr);
		}
		else {
			unresolved_pointers_.erase(id);
		}
		xml_ostream << "<" << name << " type=SharedPointer id=" + id + ">\n";
		SerializeHumanReadable("ptr", obj_shared_ptr.get());
		stream_ << "</" + name + ">\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, std::vector<T>& obj_vec)
	{
		std::size_t id = IdForObject((void*)&obj_vec);
		if (!id) {
			id = Store((void*)&obj_vec);
		}
		else {
			unresolved_pointers_.erase(id);
		}
		xml_ostream << "<" << name << " type=Vector id=" + id + ">\n";
		xml_ostream << "<Count>" << obj_vec.size() << "</Count>\n";
		for (T& object : obj_vec) {
			SerializeHumanReadable(xml_ostream, "Element", object);
		}
		xml_ostream << "</" + name + ">\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::ostream& xml_ostream, std::string name, T& object)
	{
		std::size_t id = IdForObject((void*)&object);
		if (!id) {
			id = Store((void*)&object);
		}
		else {
			// We have already visited and serialized this object. This is a cycle. Stop recursing.
			xml_ostream << "<" << name << " id=" << id << ">\n";
			xml_ostream << unresolved_pointer_ostreams_[id];
			xml_ostream << "</" << name << ">\n";

			unresolved_pointer_ostreams_.erase(id);
			return;
		}

		if (std::is_pointer<T>::value) {
			std::size_t pointee_id = IdForObject((void*)object);
			if (!id) {
				pointee_id = Store((void*)object);
				// Serialize object being pointed to. Does not handle the case of a pointer to an array of objects.
				std::ostream temp_ostream;

				ISerializable* object_serializable = dynamic_cast<ISerializable*>(object);
				if (serializable) {
					// TODO: handle scenarios when object is std::vector<>* or std::shared_ptr<>*.  
					object_serializable->SerializeHumanReadable(this, temp_ostream);
				}
				else {
					temp_ostream << *object;
				}

				unresolved_pointer_ostreams_.insert({ pointee_id , temp_ostream });
			}
			xml_ostream << "<" << name << " type=pointer id=" << id << " pointee_id=" << pointee_id << " />\n";
		}
		else {
			xml_ostream << "<" << name << " id=" << id << ">\n";

			ISerializable* object_serializable = dynamic_cast<ISerializable*>(object);
			if (serializable) {
				object_serializable->SerializeHumanReadable(this, xml_ostream);
			}
			else {
				xml_ostream << object;
			}
			xml_ostream << "</" << name << ">\n";
		}
	}
};