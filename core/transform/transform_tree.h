#pragma once

#include <vector>

#include "transform.h"

struct TransformNode {
	// If parent_index is -1, then this is the root.
	std::size_t parent_index;
	Transform transform;
}

class TransformTree {



private:
	//
	std::vector<Transform> transforms_;
};