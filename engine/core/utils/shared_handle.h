#pragma once

#include <iostream>
#include <vector>
#include <queue>

typedef std::size_t HandleID;

template <typename T>
class SharedHandle 
{
private:
	static std::vector<T> objects_;
	static std::vector <std::size_t> physical_to_virtual_map_;
	static std::vector<std::size_t> virtual_to_physical_map_;
	static std::queue<std::size_t> free_slots_;
	
	std::size_t virtual_index_;
	uint32_t ref_count_;
public:

	SharedHandle(T& object) 
	{
		ref_count_ += 1;
		//TODO: reference counting
		if (free_slots_.size() > 0) {
			virtual_index_ = free_slots_.front();
			free_slots_.pop();
			objects_[virtual_to_physical_map_[virtual_index_]] = object;
		}
		else {
			virtual_index_ = virtual_to_physical_map_.size();
			virtual_to_physical_map_.push_back(objects_.size());
			objects_.push_back(object);
		}
	}

	~SharedHandle() 
	{
		ref_count_ -= 1;
		if (ref_count == 0) {
			std::size_t physical_index_ = virtual_to_physical_map_[virtual_index_];
			if (physical_index_ < objects_.size() - 1) {
				objects_[physical_index_] = objects_.end();
				virtual_to_physical_map_[physical_to_virtual_map_.end()] = physical_index_;
			}

			objects_.pop_back();
			physical_to_virtual_map_.pop_back();
			free_slots_.push(virtual_index_);
		}
	}

	T& operator* () 
	{
		return objects_[virtual_to_physical_map_[virtual_index_]];
	}

	T* operator-> () 
	{
		return &objects_[virtual_to_physical_map_[virtual_index_]];
	}
	
	public HandleID GetHandleID() {
		return virtual_index_;
	}
};