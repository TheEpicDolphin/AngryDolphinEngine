
#include "scene_graph.h"

#include <core/transform/transform_utils.h>

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

SceneGraph::SceneGraph() {
	scene_graph_node_pool_ = new SceneGraphNode[(MAX_ENTITY_COUNT >> CHUNK_SIZE_POWER_OF_2) + 1]();
	next_pool_index_ = 0;

	// We create the world entity
	CreateEntity(glm::mat4(1.0f), -1);
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

	const std::size_t parent_pool_index = entity_to_scene_graph_node_map_[parent_id];
	SceneGraphNode& node = scene_graph_node_pool_[pool_index];
	node.type = SceneGraphNodeTypeTransform;
	node.value.transform_node = { entity_id, { transform_utils.TransformWorldToLocal(world_matrix, parent_id), world_matrix }, parent_pool_index, 0, 0, 0 };
	entity_to_scene_graph_node_map_[entity_id] = pool_index;

	return entity_id;
}

std::vector<EntityID> SceneGraph::CreateEntityChunk(std::size_t n, std::vector<glm::mat4> world_matrices = {}, std::vector<int> parent_map = {})
{
	assert(n > 0);
	assert(world_matrices.size() <= n);
	assert(parent_map.size() <= n);

	std::vector<EntityID> entity_ids;
	for (std::size_t i = 0; i < n; i++) {
		if (recycled_entity_ids_.empty()) {
			entity_ids.push_back(entity_to_scene_graph_node_map_.size());
			entity_to_scene_graph_node_map_.push_back(-1);
		}
		else {
			entity_ids.push_back(recycled_entity_ids_.front());
			recycled_entity_ids_.pop();
		}
	}

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
		pool_index = next_pool_index_;
		next_pool_index_ += n;
	}

	for (std::size_t i = 0; i < n; i++) {
		entity_to_scene_graph_node_map_[entity_ids[i]] = pool_index + i;
	}

	for (std::size_t i = 0; i < n; i++) {
		SceneGraphNode& node = scene_graph_node_pool_[pool_index + i];
		node.type = SceneGraphNodeTypeTransform;
		std::size_t parent_pool_index;
		if (parent_map[i] < 0) {
			assert(parent_map[i] >= -n);
			parent_pool_index = entity_to_scene_graph_node_map_[entity_ids[-parent_map[i] - 1]];
		}
		else {
			parent_pool_index = entity_to_scene_graph_node_map_[parent_map[i]];
		}
		TransformNode* parent = &scene_graph_node_pool_[parent_pool_index].value.transform_node;
		TransformNode* prev_sibling = parent->last_child;
		node.value.transform_node = { entity_ids[i], { transform_utils.TransformWorldToLocal(world_matrices[i], parent_id), world_matrices[i] }, parent, prev_sibling, nullptr, nullptr, nullptr };
		prev_sibling->next_sibling = &node.value.transform_node;
		if (!parent->first_child) {
			parent->first_child = &node.value.transform_node;
		}
		parent->last_child = &node.value.transform_node;
	}

	return entity_ids;
}

void SceneGraph::DestroyEntity(EntityID entity_id) 
{
	assert(entity_id > 0);
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id];
	
	std::vector<std::size_t> new_chunk_pool_indices;
	SceneGraphNode& prev_node = scene_graph_node_pool_[pool_index - 1];
	if (prev_node.type == SceneGraphNodeTypeRecycled) {
		// There is an adjacent recycled chunk before.
		const std::size_t prev_chunk_size = prev_node.value.recycled_node.chunk_size;
		const std::size_t prev_chunk_rank = prev_node.value.recycled_node.chunk_rank;
		const std::size_t prev_chunk_pool_index = recycled_chunk_map_[prev_chunk_size][prev_chunk_rank];
		for (std::size_t i = 0; i < prev_node.value.recycled_node.chunk_size; i++) {
			new_chunk_pool_indices.push_back(prev_chunk_pool_index + i);
		}
		DeleteRecycledChunkWithSwap(prev_chunk_size, prev_chunk_rank);
	}

	new_chunk_pool_indices.push_back(pool_index);
	SceneGraphNode& node = scene_graph_node_pool_[pool_index];

	// Handle removal of node from hierarchy.
	TransformNode* child = &node.value.transform_node;
	TransformNode* prev_sibling = node.value.transform_node.previous_sibling;
	TransformNode* next_sibling = node.value.transform_node.next_sibling;
	TransformNode* parent = node.value.transform_node.parent;
	if (parent->first_child == child) {
		parent->first_child = next_sibling;
	}
	if (parent->last_child == child) {
		parent->last_child = prev_sibling;
	}
	prev_sibling->next_sibling = next_sibling;
	next_sibling->previous_sibling = prev_sibling;
	
	// Set node to be recycled
	node.type = SceneGraphNodeTypeRecycled;
	
	SceneGraphNode& next_node = scene_graph_node_pool_[pool_index + 1];
	if (next_node.type == SceneGraphNodeTypeRecycled){
		// There is an adjacent recycled chunk after.
		const std::size_t next_chunk_size = next_node.value.recycled_node.chunk_size;
		const std::size_t next_chunk_rank = next_node.value.recycled_node.chunk_rank;
		const std::size_t next_chunk_pool_index = recycled_chunk_map_[next_chunk_size][next_chunk_rank];
		for (std::size_t i = 0; i < next_node.value.recycled_node.chunk_size; i++) {
			new_chunk_pool_indices.push_back(next_chunk_pool_index + i);
		}
		DeleteRecycledChunkWithSwap(next_chunk_size, next_chunk_rank);
	}

	// Create new chunk, and merge with any adjacent chunks, if any.
	const std::size_t new_chunk_size = new_chunk_pool_indices.size();
	std::size_t new_chunk_rank;
	if (recycled_chunk_map_.find(new_chunk_size) != recycled_chunk_map_.end()) {
		std::vector<std::size_t>& recycled_chunks = recycled_chunk_map_[new_chunk_size];
		new_chunk_rank = recycled_chunks.size();
		recycled_chunks.push_back(new_chunk_pool_indices[0]);
	}
	else {
		new_chunk_rank = 0;
		recycled_chunk_map_[new_chunk_size] = { new_chunk_pool_indices[0] };
	}

	for (std::size_t pool_index : new_chunk_pool_indices) {
		scene_graph_node_pool_[pool_index].value.recycled_node = { new_chunk_size, new_chunk_rank };
	}

	recycled_entity_ids_.push(entity_id);
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

void SceneGraph::DeleteRecycledChunkWithSwap(std::size_t chunk_size, std::size_t chunk_rank) {
	std::vector<std::size_t>& recycled_chunks = recycled_chunk_map_[chunk_size];
	if (recycled_chunks.size() == 1) {
		recycled_chunk_map_.erase(chunk_rank);
	}
	else {
		recycled_chunks[chunk_rank] = recycled_chunks.back();
		recycled_chunks.pop_back();
		for (std::size_t i = 0; i < chunk_size; i++) {
			scene_graph_node_pool_[recycled_chunks[chunk_rank] + i].value.recycled_node.chunk_rank = chunk_rank;
		}
	}
}
