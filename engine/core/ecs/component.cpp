#include "component.h"

/* initialized to invalid id of 0 */
template <typename T>
ComponentTypeID Component<T>::type_id_ = 0;