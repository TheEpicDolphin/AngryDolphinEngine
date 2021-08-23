#pragma once

#include <vector>
#include <core/transform/transform.h>

class SceneGraph {
public:
	SceneGraph();

	void CreateTransform(TransformID id);

	void DestroyTransform(TransformID id);
	
	const glm::mat4& GetLocalTransform(TransformID id);

	void SetLocalTransform(TransformID id, glm::mat4& local_matrix);

	const glm::mat4& GetWorldTransform(TransformID id);

	void SetWorldTransform(TransformID id, glm::mat4& world_matrix);

	const TransformID& GetParent(TransformID id);

	void SetParent(TransformID id, TransformID parent_id);

private:
	const struct TransformIndex {
		// Pointer to array of children it belongs to.
		Transform* array_ptr;
		// Offset from beginning of children array.
		std::size_t offset;
	};

	friend void TransformArrayDidReallocate(void* context, Transform* data_ptr, std::size_t size);

	// Maps transform IDs to transform indices. 
	std::vector<TransformIndex> transform_map_;
	// World transform
	Transform world_transform_;
};