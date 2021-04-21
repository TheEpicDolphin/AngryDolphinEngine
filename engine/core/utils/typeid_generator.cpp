#include "typeid_generator.h"

/* 0 is reserved for invalid id */
template <typename T>
TypeID TypeIDGenerator<T>::id_count_ = 1;