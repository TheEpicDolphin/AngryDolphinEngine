#pragma once

#include <algorithm> 

#define SCALE_THREE_HALVES(n) (n + (n >> 1))

template<typename T>
class SmartArray 
{
public:

	typedef T* iterator;
	typedef void (*ReallocationCallback)(void* context, T* data_ptr, std::size_t size);

	SmartArray() 
	{
		capacity_ = 1;
		data_ = new T[capacity_];
	}

	SmartArray(void* context, ReallocationCallback realloc_callback) : SmartArray()
	{
		callback_context_ = context;
		realloc_callback_ = realloc_callback;
	}

	SmartArray(std::size_t size) 
	{
		size_ = size;
		capacity_ = std::max(1, SCALE_THREE_HALVES(size_));
		data_ = new T[capacity_];
	}

	SmartArray(std::size_t size, void* context, ReallocationCallback realloc_callback) : SmartArray(size)
	{
		callback_context_ = context;
		realloc_callback_ = realloc_callback;
	}

	~SmartArray() 
	{
		delete[] data_;
	}

	void Push(T object) 
	{
		size_++;
		if (size > capacity_) {
			// Reallocate array of data. Scale capacity by a factor of 1.5.
			capacity_ = SCALE_THREE_HALVES(size_);
			
			T* new_data = new T[capacity_];
			std::copy(data_, data_ + size_ - 1, new_data);
			delete[] data_;
			data_ = new_data;
			data_[size_] = object;
			realloc_callback_(callback_context_, data_, size_);
		}
		else {
			data_[size_] = object;
		}
	}

	void Pop() 
	{
		if (size_ == 0) {
			// Throw error/warning.
			return;
		}

		size_--;
		delete data_[size_];
	}

	void Resize(std::size_t size) 
	{
		size_ = size;
	}

	void Reserve(std::size_t capacity) 
	{
		if (capacity < size_) {
			// Throw error/warning. It is dumb to do this.
			return;
		}

		capacity_ = capacity;
	}

	const std::size_t& Size() {
		return size_;
	}

	T& operator[](int index) 
	{
		if (index < size_) {
			return data_[index];
		}
		else {
			// Throw error
		}
	}

	iterator begin()
	{
		return data_;
	}

	iterator end()
	{
		return data_ + size_;
	}


private:
	std::size_t capacity_;
	std::size_t size_;
	T* data_;
	void* callback_context_;
	ReallocationCallback realloc_callback_;
};