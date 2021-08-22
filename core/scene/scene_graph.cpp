
#include "scene_graph.h"

void TransformArrayDidReallocate(void *context, Transform *data_ptr, std::size_t size)
{
	// Update map from transform ids -> Transform pointers
	for (std::size_t i = 0; i < size; i++) {
		Transform transform = *(data_ptr + i);
		transform_tree_map_[transform.id].array_ptr = data_ptr;
	}
}

SceneGraph::SceneGraph() {
	world_transform_ = { 0, 0, SmartArray<Transform>(this, &TransformArrayDidReallocate), glm::mat4::identity(), glm::mat4::identity() };
}

void SceneGraph::CreateTransform(TransformID id)
{
	transform_tree_map_[id] = { &world_transform_.children.data(), world_transform_.children.size() };
	world_transform_.children.push_back({ id, 0, SmartArray<Transform>(this, &TransformArrayDidReallocate), glm::mat4::identity(), glm::mat4::identity() });
}