#pragma once

template<typename T>
class SmartArray 
{
public:
	typedef void (*ReallocationCallback)(void* context, T* data_ptr, std::size_t size);

	SmartArray();
	SmartArray(std::size_t size);
	SmartArray(void* context, ReallocationCallback realloc_callback);
	SmartArray(std::size_t size, void* context, ReallocationCallback realloc_callback);

	void Append(T object);

	void Pop();

	void Resize(std::size_t size);

	void Reserve(std::size_t capacity);

	T& operator[](int index);

private:
	std::size_t capacity_;
	std::size_t size_;
	T* data_;
	void* callback_context_;
	ReallocationCallback realloc_callback_;
};