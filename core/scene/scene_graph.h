#pragma once

#include <vector>
#include <queue>
#include <map>
#include <core/ecs/entity.h>
#include <glm/mat4x4.hpp>

#define MAX_ENTITY_COUNT 16384
#define CHUNK_SIZE_POWER_OF_2 10

class SceneGraph {
public:
	SceneGraph();

	/* Creates an entity with given world matrix transformation and parent. If parent_id is 0, the entity's parent is the world.
	*/
	EntityID CreateEntity(glm::mat4 world_matrix = glm::mat4(1.0f), EntityID parent_id = 0);

	/* Each parent_map element, x, that is < 0 is assumed to be referring to the (-x)th entity to be created.
	*  Otherwise, it is assumed to be referring to an existing entity ID.
	*/
	std::vector<EntityID> CreateEntityChunk(std::size_t n, std::vector<glm::mat4> world_matrices = {}, std::vector<int> parent_map = {});

	void DestroyEntity(EntityID entity_id);

	void DestroyEntityChunk(EntityID entity_id, std::size_t n);
	
	const glm::mat4& GetLocalTransform(EntityID id);

	void SetLocalTransform(EntityID id, glm::mat4& local_transform_matrix);

	const glm::mat4& GetWorldTransform(EntityID id);

	void SetWorldTransform(EntityID id, glm::mat4& world_transform_matrix);

	const EntityID& GetParent(EntityID id);

	void SetParent(EntityID id, EntityID parent_id);

	//void PerformBlockForEach(std::vector<EntityID> entity_ids, void (*block)(void* context, EntityID entity_id, Transform& transform));

private:

	// Using a tagged union to save space.

	enum SceneGraphNodeType {
		SceneGraphNodeTypeRecycled = 0,
		SceneGraphNodeTypeTransform,
	};

	struct RecycledNode {
		std::size_t chunk_size;
		std::size_t chunk_rank;
	};

	struct TransformNode {
		// Entity ID
		EntityID entity_id;
		// Relative to parent.
		glm::mat4 local_transform_matrix;
		// Relative to world.
		glm::mat4 world_transform_matrix;

		TransformNode* parent;
		TransformNode* previous_sibling;
		TransformNode* next_sibling;
		TransformNode* first_child;
		TransformNode* last_child;
	};

	struct SceneGraphNode {
		SceneGraphNodeType type;
		union NodeValue {
			RecycledNode recycled_node;
			TransformNode transform_node;
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

	static void SceneGraph::UpdateDescendantWorldTransformationMatrices(SceneGraph::TransformNode& root_transform_node);

	static void SceneGraph::RemoveTransformNodeFromHierarchy(TransformNode* transform_node);
};