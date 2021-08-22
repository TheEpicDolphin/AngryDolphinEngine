#pragma once

#include <vector>
#include <core/transform/transform.h>

class SceneGraph {

	SceneGraph();

	void CreateTransform(TransformID id);
	
	const glm::mat4& GetLocalTransform(TransformID id);

	void SetLocalTransform(TransformID id, glm::mat4& local_matrix);

	const glm::mat4& GetWorldTransform(TransformID id);

	void SetWorldTransform(TransformID id, glm::mat4& world_matrix);

	const TransformID& GetParent(TransformID id);

	void SetParent(TransformID id, TransformID parent_id);

private:
	struct TransformIndex {
		// Pointer to array of children it belongs to.
		Transform* array_ptr;
		// Offset from beginning of children array.
		std::size_t offset;
	};

	// Maps transform IDs to transform indices. 
	std::vector<TransformIndex> transform_tree_map_;
	// World transform
	Transform world_transform_;
};