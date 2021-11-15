#pragma once

#include <vector>
#include <queue>
#include <map>
#include <core/ecs/entity.h>
#include <core/transform/transform.h>

#define MAX_ENTITY_COUNT 16384
#define CHUNK_SIZE_POWER_OF_2 10

class SceneGraph {
public:
	SceneGraph();

	EntityID CreateEntity(glm::mat4 world_matrix = glm::mat4(1.0f), EntityID parent_id = 0);

	std::vector<EntityID> CreateEntities(std::size_t n, std::vector<glm::mat4> world_matrices = {}, std::vector<std::size_t> parent_map = {});

	void DestroyEntity(EntityID entity_id);

	void DestroyEntityGroup(EntityID id);
	
	const glm::mat4& GetLocalTransform(EntityID id);

	void SetLocalTransform(EntityID id, glm::mat4& local_matrix);

	const glm::mat4& GetWorldTransform(EntityID id);

	void SetWorldTransform(EntityID id, glm::mat4& world_matrix);

	const EntityID& GetParent(EntityID id);

	void SetParent(EntityID id, EntityID parent_id);

	void PerformBlockForEach(std::vector<EntityID> entity_ids, void (*block)(void* context, EntityID entity_id, Transform& transform));

private:

	// Using a tagged union to save space.

	enum SceneGraphNodeType {
		SceneGraphNodeTypeRecycled = 0,
		SceneGraphNodeTypeTransform,
	};

	struct SceneGraphNode {
		SceneGraphNodeType type;
		union {
			struct RecycledNode {
				std::size_t chunk_size;
				std::size_t chunk_rank;
			} recycled_node;

			struct TransformNode {
				EntityID entity_id;
				Transform transform;
				std::size_t parent;
				std::size_t previous_sibling;
				std::size_t next_sibling;
				std::size_t first_child;
			} transform_node;
		} value;
	};

	SceneGraphNode* scene_graph_node_pool_;
	std::size_t next_pool_index_;

	// A "chunk" is an interval of contiguous unused transform nodes.

	// Maps chunk size to vector of recycled chunk start indices.
	std::map<std::size_t, std::vector<std::size_t>> recycled_chunk_map_;
	// Maps Entity ID to scene graph node index.
	std::vector<std::size_t> entity_to_scene_graph_node_map_;
	// Recycled Entity IDs that can be reused.
	std::queue<EntityID> recycled_entity_ids_;

	void SceneGraph::DeleteRecycledChunkWithSwap(std::size_t chunk_size, std::size_t chunk_rank);
};