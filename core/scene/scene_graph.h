#pragma once

#include 
#include <vector>
#include <core/transform/transform.h>

class SceneGraph {

	SceneGraph() {
		world_transform_ = { 0, 0, {}, glm::mat4::identity(), glm::mat4::identity() };
	}

	void CreateTransform(TransformID id) {
		transform_tree_map_[id] = { &world_transform_.children.data(), world_transform_.children.size()};
		world_transform_.children.push_back({ id, 0, {}, glm::mat4::identity(), glm::mat4::identity() });
	}
	
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