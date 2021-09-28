#pragma once

#include <iostream>
#include <unordered_map>

class Archive 
{
public:

    // TODO create map from strings to functions that will be used in case pointers cannot be figured out. This allows
    // serializing/deserializing function pointers

    Archive() {
        id_to_object_map_.push_back(nullptr);
    }

    ostream& operator<<(ostream& os, const Date& dt)
    {
		stream_ << dt.mo << '/' << dt.da << '/' << dt.yr;
        return os;
    }

	template<typename T>
	void SerializeHumanReadable(std::string name, std::shared_ptr<T>& obj_shared_ptr)
	{
		stream_ << "<" + name + " type=SharedPointer";
		std::size_t id = IdForObject((void*)&obj_shared_ptr);
		if (!id) {
			id = Store((void*)&obj_shared_ptr);
		}
		else {
			unresolved_pointers_.erase(id);
		}
		stream_ << " id=" + id + ">\n";
		SerializeHumanReadable("ptr", obj_shared_ptr.get());
		stream_ << "</" + name + ">\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::string name, std::vector<T>& obj_vec)
	{
		stream_ << "<" + name + " type=Vector" + " count=" + obj_vec.size() + " id=" + id + "/>\n";
		std::size_t id = IdForObject((void*)&obj_vec);
		if (!id) {
			id = Store((void*)&obj_vec);
		}
		else {
			unresolved_pointers_.erase(id);
		}
		stream_ << " id=" + id + ">\n";

		for (T& object : obj_vec) {
			stream_ << "<VectorElement>\n";
			object.Serialize(self);
			stream_ << "</VectorElement>\n";
		}
		stream_ << "</" + name + ">\n";
	}

	template<typename T>
	void SerializeHumanReadable(std::string name, T& object)
	{
		if (std::is_pointer<T>::value) {
			stream_ << "<" + name + " type=Pointer";
			std::size_t id = IdForObject((void*)object);
			if (!id) {
				id = Store((void*)object);
				id_to_unresolved_pointers_.insert({ id , object });
			}
			stream_ << " ref_id=" + id + " />\n";
		}
		else {
			stream_ << "<" + name + " type=Object";
			std::size_t id = IdForObject((void*)&object);
			if (!id) {
				id = Store((void*)&object);
			}
			else {
				id_to_unresolved_pointers_.erase(id);
			}

			stream_ << " id=" + id + ">\n";
			object.Serialize(archive)
		    stream_ << "</" + name + ">\n";
		}
	}

    template<typename... Args>
    void SerializeHumanReadable(std::pair<std::string, Args&> ...args)
    {
		{ SerializeHumanReadable(args.first, args.second)... };
    }

    template<typename... Args>
    void DeserializeHumanReadable(std::pair<std::string, Args> ...args)
    {

    }

	void EvaluateUnresolvedPointers() {
		// These are probably pointers to objects allocated in the heap.
		stream_ << "<heap>\n";
		std::pair<std::size_t, Pending> unresolved_pointers = unresolved_pointers_;
		while () {

		}
		for (std::pair<std::size_t, Pending> unresolved_pointer : id_to_unresolved_pointers_) {
			unresolved_pointer.second.Serialize();
		}
		stream_ << "<\heap>\n";
	}

private:
    std::vector<void*> id_to_object_map_;
    std::unordered_map<void*, std::size_t> object_to_id_map_;
	std::unordered_map<std::size_t, Pending> unresolved_pointers_;
	std::ostream stream_;

	std::size_t Store(void* object_ptr)
	{
		const std::size_t id = id_to_object_map_.size();
		id_to_object_map_.push_back(var_ptr);
		object_to_id_map_[var_ptr] = id;
		return id;
	}

	std::size_t IdForObject(void* object_ptr)
	{
		if (object_to_id_map_.find(object_ptr) != object_to_id_map_.end()) {
			return object_to_id_map_[object_ptr];
		}
		return 0;
	}
};