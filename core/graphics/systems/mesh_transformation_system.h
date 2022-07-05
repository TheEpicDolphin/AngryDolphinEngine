#pragma once

#include <unordered_map>
#include <vector>

#include <glm/mat4x4.hpp>

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <core/scene/scene.h>
#include <core/scene/scene_graph.h>

#include "../components/mesh_renderable_component.h"
#include "../mesh.h"

class MeshTransformationSystem : 
	public ISystem,
	private MeshLifecycleEventsListener,
	private EntityTransformEventsListener
{
public:
	MeshTransformationSystem() = default;

	void Initialize(ServiceContainer service_container) override;

	void OnInstantiateEntity(ecs::EntityID entity_id) {};

	void OnCleanupEntity(ecs::EntityID entity_id) override;

	void OnFixedUpdate(double fixed_delta_time) {}

	void OnFrameUpdate(double delta_time, double alpha) override;

private:
	struct MeshTransformationState {
		Mesh* mesh_handle;
		bool is_stale;
	};

	std::unordered_map<ecs::EntityIndex, MeshTransformationState> entity_mesh_trans_state_map_;
	std::unordered_map<Mesh*, std::vector<ecs::EntityID>> mesh_to_entities_map_;

	// MeshLifecycleEventsListener
	void MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) override;
	void MeshDidDestroy(Mesh* mesh) override;

	// EntityLifecycleEventsListener
	void EntityWorldTransformDidChange(ecs::EntityID entity_id, glm::mat4 new_world_transform) override;

	void RemoveEntityFromMeshToEntitiesMapping(ecs::EntityID entity, Mesh* mesh_handle);

	ecs::Registry& component_registry_;
	SceneGraph& scene_graph_;
};
