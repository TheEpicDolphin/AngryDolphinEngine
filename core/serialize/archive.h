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
        os << dt.mo << '/' << dt.da << '/' << dt.yr;
        return os;
    }

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

private:
    std::vector<void*> id_to_object_map_;
    std::unordered_map<void*, std::size_t> object_to_id_map_;
};