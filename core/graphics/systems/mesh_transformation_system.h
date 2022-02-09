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
	private EntityLifecycleEventsListener
{
public:
	MeshTransformationSystem() = default;

	void OnFrameUpdate(double delta_time, double alpha, const IScene& scene) override;

private:
	struct MeshTransformationState {
		MeshID mesh_id;
		bool is_stale;
	};

	std::unordered_map<ecs::EntityIndex, MeshTransformationState> entity_mesh_trans_state_map_;
	std::unordered_map<MeshID, std::vector<ecs::EntityID>> mesh_to_entities_map_;

	// MeshLifecycleEventsListener
	void MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) override;
	void MeshDidDestroy(MeshID mesh_id) override;

	// EntityLifecycleEventsListener
	void EntityDidDestroy(ecs::EntityID entity_id) override;
	void EntityWorldTransformDidChange(ecs::EntityID entity_id, glm::mat4 new_world_transform) override;
};
