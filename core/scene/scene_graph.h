#pragma once

#include <vector>
#include <queue>
#include <map>
#include <core/definitions/transform/transform_service.h>
#include <core/ecs/entity.h>
#include <core/utils/event_announcer.h>
#include <glm/mat4x4.hpp>

#define MAX_ENTITY_COUNT 16384

struct EntityLifecycleEventsListener {
	virtual void EntityDidDestroy(ecs::EntityID entity_id) = 0;
	virtual void EntityWorldTransformDidChange(ecs::EntityID entity_id, glm::mat4 new_world_transform) = 0;
};

class SceneGraph : public ITransformService {
public:
	SceneGraph();

	/* Creates an entity with given world matrix transformation and parent. If parent_id is 0, the entity's parent is the world.
	*/
	ecs::EntityID CreateEntity(glm::mat4 world_matrix = glm::mat4(1.0f), ecs::EntityID parent_id = {});

	/* Each parent_map element, x, that is < 0 is assumed to be referring to the (-x)th entity to be created.
	*  Otherwise, it is assumed to be referring to an existing entity ID.
	*/
	std::vector<ecs::EntityID> CreateEntityChunk(std::size_t n, std::vector<glm::mat4> world_matrices = {}, std::vector<int> parent_map = {});

	void DestroyEntity(ecs::EntityID entity_id);

	void DestroyEntityChunk(ecs::EntityID entity_id, std::size_t n);

	bool IsValid(ecs::EntityID entity_id);

	void AddLifecycleEventsListenerForEntity(EntityLifecycleEventsListener* listener, ecs::EntityID entity_id);

	void RemoveLifecycleEventsListenerForEntity(EntityLifecycleEventsListener* listener, ecs::EntityID entity_id);
	
	const glm::mat4& GetLocalTransform(ecs::EntityID entity_id) const override;

	void SetLocalTransform(ecs::EntityID entity_id, glm::mat4& local_transform_matrix) const override;

	const glm::mat4& GetWorldTransform(ecs::EntityID entity_id) const override;

	void SetWorldTransform(ecs::EntityID entity_id, glm::mat4& world_transform_matrix) const override;

	const ecs::EntityID& GetParent(ecs::EntityID entity_id) const override;

	void SetParent(ecs::EntityID entity_id, ecs::EntityID parent_id) const override;

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
		ecs::EntityID entity_id;
		// Relative to parent.
		glm::mat4 local_transform_matrix;
		// Relative to world.
		glm::mat4 world_transform_matrix;

		TransformNode* parent;
		TransformNode* previous_sibling;
		TransformNode* next_sibling;
		TransformNode* first_child;
		TransformNode* last_child;
		EventAnnouncer<EntityLifecycleEventsListener>* lifecycle_events_announcer;
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
	// Maps Entity index to scene graph node index.
	std::vector<std::size_t> entity_to_scene_graph_node_map_;
	// Recycled Entity indices that can be reused.
	std::queue<ecs::EntityID> recycled_entity_ids_;

	void DeleteRecycledChunkWithSwap(std::size_t chunk_size, std::size_t chunk_rank);

	static void UpdateDescendantWorldTransformationMatrices(TransformNode& root_transform_node);

	static void RemoveTransformNodeFromHierarchy(TransformNode* transform_node);

	static void SetWorldTransformMatrix(TransformNode* transform_node, glm::mat4 new_world_matrix);
};