
#include "scene_graph.h"

#include <core/transform/transform.h>

#include <iostream>

using namespace ecs;

SceneGraph::SceneGraph() {
	scene_graph_node_pool_ = new SceneGraphNode[MAX_ENTITY_COUNT]();
	next_pool_index_ = 0;

	// We create the world entity
	CreateEntity(glm::mat4(1.0f), { 0, 0 });
}

EntityID SceneGraph::CreateEntity(glm::mat4 world_matrix, EntityID parent_id)
{
	return CreateEntityChunk(1, { world_matrix }, { (int)parent_id.index })[0];
}

std::vector<EntityID> SceneGraph::CreateEntityChunk(std::size_t n, std::vector<glm::mat4> world_matrices, std::vector<int> parent_map)
{
	assert(n > 0);
	assert(world_matrices.size() <= n);
	assert(parent_map.size() <= n);

	std::vector<EntityID> entity_ids;
	for (std::size_t i = 0; i < n; i++) {
		if (recycled_entity_ids_.empty()) {
			entity_ids.push_back({ 0, (EntityIndex)entity_to_scene_graph_node_map_.size() });
			entity_to_scene_graph_node_map_.push_back(-1);
		}
		else {
			EntityID recycled_entity_id = recycled_entity_ids_.front();
			// We must increase the version number every time this entity id is reused!
			entity_ids.push_back({ ++recycled_entity_id.version, recycled_entity_id.index });
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
		entity_to_scene_graph_node_map_[entity_ids[i].index] = pool_index + i;
	}

	for (std::size_t i = 0; i < n; i++) {
		SceneGraphNode& node = scene_graph_node_pool_[pool_index + i];
		node.type = SceneGraphNodeTypeTransform;
		std::size_t parent_pool_index;
		if (parent_map[i] < 0) {
			assert(parent_map[i] >= -((int)n));
			parent_pool_index = entity_to_scene_graph_node_map_[entity_ids[-parent_map[i] - 1].index];
		}
		else {
			parent_pool_index = entity_to_scene_graph_node_map_[parent_map[i]];
		}
		
		TransformNode* parent = &scene_graph_node_pool_[parent_pool_index].value.transform_node;
		TransformNode* prev_sibling = parent->last_child;
		node.value.transform_node = 
		{ 
			entity_ids[i],
			transform::TransformWorldToLocal(parent->world_transform_matrix, world_matrices[i]),
			world_matrices[i], 
			parent, 
			prev_sibling, 
			nullptr, 
			nullptr, 
			nullptr 
		};

		if (prev_sibling) {
			prev_sibling->next_sibling = &node.value.transform_node;
		}
		if (!parent->first_child) {
			parent->first_child = &node.value.transform_node;
		}
		parent->last_child = &node.value.transform_node;
	}

	return entity_ids;
}

void SceneGraph::DestroyEntity(EntityID entity_id)
{
	DestroyEntityChunk(entity_id, 1);
}

void SceneGraph::DestroyEntityChunk(EntityID entity_id, std::size_t n)
{
	assert(entity_id != null_entity_id);
	const std::size_t chunk_start_pool_index = entity_to_scene_graph_node_map_[entity_id.index];

	std::vector<std::size_t> new_chunk_pool_indices;
	SceneGraphNode& prev_node = scene_graph_node_pool_[chunk_start_pool_index - 1];
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

	// We officially destroy the entities here
	for (std::size_t i = 0; i < n; i++) {
		const std::size_t pool_index = chunk_start_pool_index + i;
		new_chunk_pool_indices.push_back(pool_index);
		SceneGraphNode& node = scene_graph_node_pool_[pool_index];

		// Handle removal of node from hierarchy.
		RemoveTransformNodeFromHierarchy(&node.value.transform_node);

		// Set node to be recycled
		node.type = SceneGraphNodeTypeRecycled;

		// Recycle entity id.
		recycled_entity_ids_.push(node.value.transform_node.entity_id);
	}
	
	SceneGraphNode& next_node = scene_graph_node_pool_[chunk_start_pool_index + n];
	if (next_node.type == SceneGraphNodeTypeRecycled) {
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
}

bool SceneGraph::IsValid(ecs::EntityID entity_id) {
	if (entity_id.index >= entity_to_scene_graph_node_map_.size()) {
		return false;
	}
	SceneGraph::SceneGraphNode& node = scene_graph_node_pool_[entity_to_scene_graph_node_map_[entity_id.index]];
	return node.type == SceneGraphNodeTypeTransform && node.value.transform_node.entity_id.version == entity_id.version;
}

void SceneGraph::AddLifecycleEventsListenerForEntity(EntityTransformEventsListener* listener, ecs::EntityID entity_id) {
	if (IsValid(entity_id)) {
		TransformNode& transform_node = scene_graph_node_pool_[entity_to_scene_graph_node_map_[entity_id.index]].value.transform_node;
		transform_node.transform_events_announcer = new EventAnnouncer<EntityTransformEventsListener>();
		transform_node.transform_events_announcer->AddListener(listener);
	}
}

void SceneGraph::RemoveLifecycleEventsListenerForEntity(EntityTransformEventsListener* listener, ecs::EntityID entity_id) {
	if (IsValid(entity_id)) {
		TransformNode& transform_node = scene_graph_node_pool_[entity_to_scene_graph_node_map_[entity_id.index]].value.transform_node;
		if (transform_node.transform_events_announcer) {
			transform_node.transform_events_announcer->RemoveListener(listener);
		}
	}
}

const glm::mat4& SceneGraph::GetLocalTransform(EntityID entity_id) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	return scene_graph_node_pool_[pool_index].value.transform_node.local_transform_matrix;
}

void SceneGraph::SetLocalTransform(EntityID entity_id, glm::mat4& local_transform_matrix) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	TransformNode& transform_node = scene_graph_node_pool_[pool_index].value.transform_node;
	if (local_transform_matrix != transform_node.local_transform_matrix) {
		transform_node.local_transform_matrix = local_transform_matrix;
		const TransformNode* parent_transform_node = transform_node.parent;
		SetWorldTransformMatrix(&transform_node, transform::TransformLocalToWorld(transform_node.parent->world_transform_matrix, local_transform_matrix));
		UpdateDescendantWorldTransformationMatrices(transform_node);
	}
}

const glm::mat4& SceneGraph::GetWorldTransform(EntityID entity_id) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	return scene_graph_node_pool_[pool_index].value.transform_node.world_transform_matrix;
}

void SceneGraph::SetWorldTransform(EntityID entity_id, glm::mat4& world_transform_matrix) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	TransformNode& transform_node = scene_graph_node_pool_[pool_index].value.transform_node;
	if (world_transform_matrix != transform_node.world_transform_matrix) {
		SetWorldTransformMatrix(&transform_node, world_transform_matrix);
		const TransformNode* parent_transform_node = transform_node.parent;
		transform_node.local_transform_matrix = transform::TransformWorldToLocal(transform_node.parent->world_transform_matrix, world_transform_matrix);
		UpdateDescendantWorldTransformationMatrices(transform_node);
	}
}

const EntityID& SceneGraph::GetParent(EntityID entity_id) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	TransformNode& transform_node = scene_graph_node_pool_[pool_index].value.transform_node;
	return transform_node.parent->entity_id;
}

void SceneGraph::SetParent(EntityID entity_id, EntityID parent_id) const
{
	const std::size_t pool_index = entity_to_scene_graph_node_map_[entity_id.index];
	TransformNode& transform_node = scene_graph_node_pool_[pool_index].value.transform_node;
	// Remove transform node from previous parent
	RemoveTransformNodeFromHierarchy(&transform_node);

	const std::size_t new_parent_pool_index = entity_to_scene_graph_node_map_[parent_id.index];
	TransformNode& new_parent_transform_node = scene_graph_node_pool_[new_parent_pool_index].value.transform_node;

	// Add transform node to new parent
	TransformNode* prev_sibling = new_parent_transform_node.last_child;
	prev_sibling->next_sibling = &transform_node;
	if (!new_parent_transform_node.first_child) {
		new_parent_transform_node.first_child = &transform_node;
	}
	new_parent_transform_node.last_child = &transform_node;
	// The entity's world transformation matrix stays the same when re-parented. However, its local
	// local transformation matrix is updated to reflect the new parenting.
	transform_node.local_transform_matrix = transform::TransformWorldToLocal(new_parent_transform_node.world_transform_matrix, transform_node.world_transform_matrix);
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

void SceneGraph::UpdateDescendantWorldTransformationMatrices(TransformNode& root_transform_node)
{
	std::queue<TransformNode*> bfs_descendant_queue;
	bfs_descendant_queue.push(&root_transform_node);
	while (!bfs_descendant_queue.empty()) {
		const TransformNode* transform_node = bfs_descendant_queue.front();
		bfs_descendant_queue.pop();
		const glm::mat4& parent_world_matrix = transform_node->world_transform_matrix;
		TransformNode* child_transform_node = transform_node->first_child;
		while (child_transform_node != nullptr) {
			SetWorldTransformMatrix(child_transform_node, child_transform_node->local_transform_matrix * parent_world_matrix);
			bfs_descendant_queue.push(child_transform_node);
			child_transform_node = child_transform_node->next_sibling;
		}
	}
}

void SceneGraph::RemoveTransformNodeFromHierarchy(TransformNode* transform_node)
{
	TransformNode* prev_sibling = transform_node->previous_sibling;
	TransformNode* next_sibling = transform_node->next_sibling;
	TransformNode* parent = transform_node->parent;
	if (parent->first_child == transform_node) {
		parent->first_child = next_sibling;
	}
	if (parent->last_child == transform_node) {
		parent->last_child = prev_sibling;
	}
	prev_sibling->next_sibling = next_sibling;
	next_sibling->previous_sibling = prev_sibling;
}

void SceneGraph::SetWorldTransformMatrix(TransformNode* transform_node, glm::mat4 new_world_matrix) {
	transform_node->world_transform_matrix = new_world_matrix;
	if (transform_node->transform_events_announcer) {
		transform_node->transform_events_announcer->Announce(&EntityTransformEventsListener::EntityWorldTransformDidChange, transform_node->entity_id, new_world_matrix);
	}
}
