#pragma once

#include <vector>
#include <queue>
#include <core/ecs/entity.h>
#include <core/transform/transform.h>

#define MAX_ENTITY_COUNT 16384
#define CHUNK_SIZE_POWER_OF_2 10

class SceneGraph {
public:
	SceneGraph();

	EntityID CreateEntity(glm::vec3 position = glm::vec3(0.0f), EntityID parent_id = 0);

	void DestroyEntity(EntityID id);
	
	const glm::mat4& GetLocalTransform(EntityID id);

	void SetLocalTransform(EntityID id, glm::mat4& local_matrix);

	const glm::mat4& GetWorldTransform(EntityID id);

	void SetWorldTransform(EntityID id, glm::mat4& world_matrix);

	const EntityID& GetParent(EntityID id);

	void SetParent(EntityID id, EntityID parent_id);

private:

	const struct TransformChunkNode {
		TransformChunkNode* prev;
		TransformChunkNode* next;
		Transform transforms[1 << CHUNK_SIZE_POWER_OF_2];
		std::uint16_t count;
	};

	const struct TransformIndex 
	{
		std::uint16_t chunk_index;
		std::uint16_t offset;
	};

	TransformChunkNode* transform_chunk_node_pool_;
	std::queue<std::size_t> recycled_chunk_node_indices_;
	std::vector<std::size_t> entity_transform_key_map_;
	std::size_t largest_node_index_;

	std::queue<EntityID> recycled_entity_ids_;

	Transform& SceneGraph::TransformForKey(std::size_t key);
};