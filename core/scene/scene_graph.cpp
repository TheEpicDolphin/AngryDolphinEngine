
#include "scene_graph.h"

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

TransformIndex TransformIndexForKey(std::size_t key)
{
	const std::uint16_t chunk_index = key >> CHUNK_SIZE_POWER_OF_2;
	const std::uint16_t offset = key - chunk_index;
	return { chunk_index, offset };
}

TransformIndex AdvanceIndex(Transform index, std::size_t n) 
{

}

SceneGraph::SceneGraph() {
	scene_graph_node_pool_ = new SceneGraphNode[(MAX_ENTITY_COUNT >> CHUNK_SIZE_POWER_OF_2) + 1]();
	next_pool_index_ = 0;

	// We create the world entity
	CreateEntity(glm::vec3(0, 0, 0), -1);
}

EntityID SceneGraph::CreateEntity(glm::mat4 world_matrix = glm::mat4(1.0f), EntityID parent_id = 0)
{
	EntityID entity_id;
	if (recycled_entity_ids_.empty()) {
		entity_id = entity_to_scene_graph_node_map_.size();
		entity_to_scene_graph_node_map_.push_back(-1);
	}
	else {
		entity_id = recycled_entity_ids_.front();
		recycled_entity_ids_.pop();
	}

	const std::size_t parent_pool_index = entity_to_scene_graph_node_map_[parent_id];
	std::map<std::size_t, std::vector<std::size_t>>::iterator iter_begin = recycled_chunk_map_.begin();

	std::size_t pool_index;
	if (iter_begin != recycled_chunk_map_.end()) {
		const std::size_t min_chunk_size = iter_begin->first;
		assert(min_chunk_size > 0);

		pool_index = iter_begin->second.back();
		iter_begin->second.pop_back();

		if (min_chunk_size > 1) {
			// If the minimum chunk size found was greater than one, we will split 
			recycled_chunk_map_.insert(iter_begin, { min_chunk_size - 1, { pool_index + 1 } });
		}

		if (iter_begin->second.empty()) {
			// If there are no more chunks with size == min_chunk_size, erase it from the recycled chunk map.
			recycled_chunk_map_.erase(iter_begin);
		}
	}
	else {
		pool_index = next_pool_index_++;
	}

	SceneGraphNode& node = scene_graph_node_pool_[pool_index];
	node.type = SceneGraphNodeTypeTransform;
	node.value.transform_node = { entity_id, { TransformUtils.WorldToLocal(world_matrix, parent_id), world_matrix }, parent_pool_index, 0, 0, 0 };
	entity_to_scene_graph_node_map_[entity_id] = pool_index;

	return entity_id;
}

std::vector<EntityID> SceneGraph::CreateEntities(std::size_t n, std::vector<glm::mat4> world_matrices = glm::mat4(1.0f), std::vector<std::size_t> parent_map)
{
	EntityID entity_id;
	if (recycled_entity_ids_.empty()) {
		entity_id = entity_to_scene_graph_node_map_.size();
		entity_to_scene_graph_node_map_.push_back(-1);
	}
	else {
		entity_id = recycled_entity_ids_.front();
		recycled_entity_ids_.pop();
	}

	const std::size_t parent_pool_index = entity_to_scene_graph_node_map_[parent_id];
	std::map<std::size_t, std::vector<std::size_t>>::iterator iter_begin;
	if (n == 1) {
		iter_begin = recycled_chunk_map_.begin();
	}
	else {
		iter_begin = recycled_chunk_map_.lower_bound(n);
	}
	
	std::size_t pool_index;
	if (iter_begin != recycled_chunk_map_.end()) {
		const std::size_t min_chunk_size = iter_begin->first;
		assert(min_chunk_size > 0);

		pool_index = iter_begin->second.back();
		iter_begin->second.pop_back();

		if (min_chunk_size > n) {
			// If the minimum chunk size found was greater than n, we will split 
			recycled_chunk_map_.insert(iter_begin, { min_chunk_size - n, { pool_index + n } });
		}

		if (iter_begin->second.empty()) {
			// If there are no more chunks with size == min_chunk_size, erase it from the recycled chunk map.
			recycled_chunk_map_.erase(iter_begin);
		}
	}
	else {
		pool_index = next_pool_index_++;
	}

	return entity_id;
}

void SceneGraph::DestroyEntity(EntityID id) 
{
	
	recycled_entity_ids_.push(id);
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

const EntityID& SceneGraph::GetParent(Entity id) 
{
	const std::size_t transform_key = entity_transform_key_map_[id];
	const Transform& transform = TransformForKey(transform_key);
	return transform.parent_id;
}

void SceneGraph::SetParent(EntityID id, EntityID parent_id)
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

Transform& SceneGraph::TransformForKey(std::size_t key)
{
	const TransformIndex transform_index = TransformIndexForKey(key);
	return transform_chunk_node_pool_[transform_index.chunk_index]->transforms[transform_index.offset];
}

Transform& SceneGraph::TransformForIndex(TransformIndex index)
{
	return transform_chunk_node_pool_[index.chunk_index]->transforms[index.offset];
}

std::size_t SceneGraph::InsertTransformAtIndex(TransformIndex index, Transform transform)
{
	TransformChunkNode* chunk_node = transform_chunk_node_pool_[index.chunk_index];
	
}

void SceneGraph::RemoveTransformForKey(std::size_t key)
{
	TransformChunkNode* chunk_node = transform_chunk_node_pool_[index.chunk_index];

}

TransformIndex SceneGraph::ChildEndIndexForParent(EntityID parent_id)
{
	const std::size_t parent_key = entity_transform_key_map_[parent_id];
	Transform& parent_transform = TransformForKey(parent_key);

	if (parent_transform.child_count == 0) {
		const TransformIndex parent_transform_index = TransformIndexForKey(parent_key)
		TransformIndex previous_sibling_index= AdvanceIndex(parent_transform_index, -1);
		Transform previous_sibling_transform = TransformForIndex(previous_sibling_index);
		while (previous_sibling_transform.child_count == 0 && previous_sibling_transform.parent_id == parent_transform.parent_id) {
			previous_sibling_index = AdvanceIndex(previous_sibling_index, -1);
			previous_sibling_transform = TransformForIndex(previous_sibling_index);
		}

		if (previous_sibling_transform.parent_id != parent_transform.parent_id) {
			return AdvanceIndex(parent_transform_index, 1);
		}
		else {
			return AdvanceIndex(previous_sibling_transform.first_child_id, previous_sibling_transform.child_count + 1);
		}
	}
	else {
		const std::size_t first_child_key = entity_transform_key_map_[parent_transform.first_child_id];
		const TransformIndex first_child_index = TransformIndexForKey(first_child_key);
		return AdvanceIndex(first_child_index, parent_transform.child_count + 1);
	}
}
