
#include "smart_array.h"

SmartArray::SmartArray() 
{

}

SmartArray::SmartArray(std::size_t size) 
{

}

SmartArray::SmartArray(void* context, ReallocationCallback realloc_callback) 
{

}

SmartArray::SmartArray(std::size_t size, void* context, ReallocationCallback realloc_callback) 
{

}

void SmartArray::Append(T object) 
{
	// Use std::copy to copy data from old array to new

	// After reallocation, call realloc_callback_.
}

void SmartArray::Pop() 
{

}

void SmartArray::Resize(std::size_t size) 
{

}

void SmartArray::Reserve(std::size_t capacity) 
{

}

T& SmartArray::operator[](int index) 
{

}