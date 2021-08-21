#pragma once

#include <vector>
#include <core/transform/transform.h>
#include <core/transform/transform_tree.h>

class SceneGraph {
	
	const glm::mat4& GetLocalTransform(TransformID id);

	void SetLocalTransform(TransformID id, glm::mat4& local_matrix);

	const glm::mat4& GetWorldTransform(TransformID id);

	void SetWorldTransform(TransformID id, glm::mat4& world_matrix);

	const TransformID& GetParent(TransformID id);

	void SetParent(TransformID id, TransformID parent_id);

private:
	// Maps transform IDs to trees
	std::vector<std::shared_ptr<TransformTree>> transform_tree_map_;
};