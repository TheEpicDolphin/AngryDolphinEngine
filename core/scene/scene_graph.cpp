
#include "scene_graph.h"

#include <queue>

void UpdateDescendantWorldMatrices(Transform* root_transform)
{
	std::queue<Transform*> bfs_descendant_queue;
	bfs_descendant_queue.push(root_transform);
	while (!bfs_descendant_queue.empty()) {
		Transform* transform = bfs_descendant_queue.front();
		bfs_descendant_queue.pop();
		const glm::mat4& parent_world_matrix = transform->world_matrix;
		for (SmartArray<Transform>::iterator it = transform->children.begin(); it != transform->children.end(); ++it) {
			it->world_matrix = parent_world_matrix * it->local_matrix;
			bfs_descendant_queue.push(it);
		}
	}
}

void TransformArrayDidReallocate(void *context, Transform* data_ptr, std::size_t size)
{
	SceneGraph* scene_graph = static_cast<SceneGraph*>(context);
	// Update map from transform ids -> Transform pointers
	for (std::size_t i = 0; i < size; i++) {
		Transform* transform = data_ptr + i;
		scene_graph->transform_map_[transform->id] = {data_ptr, i};
	}
}

SceneGraph::SceneGraph() {
	world_transform_ = { 0, 0, SmartArray<Transform>(this, &TransformArrayDidReallocate), glm::mat4(1.0f), glm::mat4(1.0f) };
}

void SceneGraph::CreateTransform(TransformID id)
{
	transform_map_[id] = { world_transform_.children.begin(), world_transform_.children.Size() };
	world_transform_.children.Push({ id, 0, SmartArray<Transform>(this, &TransformArrayDidReallocate), glm::mat4(1.0f), glm::mat4(1.0f) });
}

void SceneGraph::DestroyTransform(TransformID id) 
{
	const TransformIndex& transform_index = transform_map_[id];

	//world_transform_.children[];
	transform_map_[id] = {};
}

const glm::mat4& SceneGraph::GetLocalTransform(TransformID id) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		return (*(transform_index.array_ptr + transform_index.offset)).local_matrix;
	}
	else {
		// Throw error/warning
	}
}

void SceneGraph::SetLocalTransform(TransformID id, glm::mat4& local_matrix) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		Transform* transform = transform_index.array_ptr + transform_index.offset;
		const TransformIndex& parent_transform_index = transform_map_[transform->parent_id];
		Transform* parent_transform = (parent_transform_index.array_ptr + parent_transform_index.offset);
		transform->world_matrix = parent_transform->world_matrix * local_matrix;
		transform->local_matrix = local_matrix;
		UpdateDescendantWorldMatrices(transform);
	}
	else {
		// Throw error/warning
	}
}

const glm::mat4& SceneGraph::GetWorldTransform(TransformID id) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		return (*(transform_index.array_ptr + transform_index.offset)).world_matrix;
	}
	else {
		// Throw error/warning
	}
}

void SceneGraph::SetWorldTransform(TransformID id, glm::mat4& world_matrix) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		Transform* transform = transform_index.array_ptr + transform_index.offset;
		const TransformIndex& parent_transform_index = transform_map_[transform->parent_id];
		Transform* parent_transform = (parent_transform_index.array_ptr + parent_transform_index.offset);
		transform->world_matrix = world_matrix;
		transform->local_matrix = glm::inverse(parent_transform->world_matrix) * world_matrix;
		UpdateDescendantWorldMatrices(transform);
	}
	else {
		// Throw error/warning
	}
}

const TransformID& SceneGraph::GetParent(TransformID id) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		return (*(transform_index.array_ptr + transform_index.offset)).parent_id;
	}
	else {
		// Throw error/warning
	}
}

void SceneGraph::SetParent(TransformID id, TransformID parent_id) 
{
	const TransformIndex& transform_index = transform_map_[id];
	if (transform_index.array_ptr) {
		Transform* transform = transform_index.array_ptr + transform_index.offset;

		// Deal with old parent.
		const TransformIndex& old_parent_transform_index = transform_map_[transform->parent_id];
		Transform* old_parent_transform = (old_parent_transform_index.array_ptr + old_parent_transform_index.offset);

		const TransformIndex& new_parent_transform_index = transform_map_[parent_id];
		Transform* new_parent_transform = (new_parent_transform_index.array_ptr + new_parent_transform_index.offset);
		transform->parent_id = parent_id;
		transform->world_matrix = new_parent_transform->world_matrix * transform->local_matrix;
		UpdateDescendantWorldMatrices(transform);
	}
	else {
		// Throw error/warning
	}
}
