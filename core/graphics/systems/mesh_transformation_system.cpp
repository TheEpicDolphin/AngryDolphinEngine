
#include "mesh_transformation_system.h"

#include <glm/gtc/type_ptr.hpp>
#include <core/transform/transform.h>

void MeshTransformationSystem::OnFrameUpdate(double delta_time, double alpha, const IScene& scene)
{
	// Iterate through mesh renderables and calculate world mesh bounds
	std::function<void(ecs::EntityID, MeshRenderableComponent&)> mesh_renderables_block =
		[this, &scene](ecs::EntityID entity_id, MeshRenderableComponent& mesh_rend) {
		if (mesh_rend.enabled) {
			std::shared_ptr<Mesh> mesh = mesh_rend.shared_mesh;
			const MeshID mesh_id = mesh->GetInstanceID();
			std::unordered_map<ecs::EntityIndex, MeshTransformationState>::iterator mesh_trans_state_iter = entity_mesh_trans_state_map_.find(entity_id.index);
			bool calculate_mesh_bounds;
			if (mesh_trans_state_iter != entity_mesh_trans_state_map_.end()) {
				calculate_mesh_bounds = mesh_id != mesh_trans_state_iter->second.mesh_id || mesh_trans_state_iter->second.is_stale;
			}
			else {
				calculate_mesh_bounds = true;
			}

			std::unordered_map<MeshID, std::vector<ecs::EntityID>> mesh_entities_iter = this->mesh_to_entities_map_.find(mesh_id);
			if (mesh_entities_iter != this->mesh_to_entities_map_.end()) {
				mesh_entities_iter->second.push_back(entity_id);
			}
			else {
				mesh_to_entities_map_[mesh_id] = { entity_id };
				mesh->AddLifecycleEventsListener(this);
			}

			// TODO: Calculate mesh bounds in world
			if (calculate_mesh_bounds) {
				const std::vector<glm::vec3>& local_vert_positions = mesh_rend.mesh->GetVertexPositions();
				glm::mat4 entity_transform = scene.TransformGraph().GetWorldTransform(entity_id);
				glm::vec3 min_p = transform::TransformPointLocalToWorld(local_vert_positions[0], entity_transform);
				glm::vec3 max_p = min_p;
				for (std::size_t i = 1; i < local_vert_positions.size(); i++) {
					glm::vec3 world_p = transform::TransformPointLocalToWorld(local_vert_positions[i], entity_transform);
					min_p = glm::min(min_p, world_p);
					max_p = glm::max(max_p, world_p);
				}
				mesh_rend.world_mesh_bounds_ = geometry::Bounds(min_p, max_p);
			}

			entity_mesh_trans_state_map_[entity_id.index] = { mesh_id, false };

		}
	};
	scene.ComponentRegistry().EnumerateComponentsWithBlock<MeshRenderableComponent>(mesh_renderables_block);
}

// MeshLifecycleEventsListener
void MeshTransformationSystem::MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) {
	
}

void MeshTransformationSystem::MeshDidDestroy(MeshID mesh_id) {

}

// EntityLifecycleEventsListener
void MeshTransformationSystem::EntityDidDestroy(ecs::EntityID entity_id) {

}

void MeshTransformationSystem::EntityWorldTransformDidChange(ecs::EntityID entity_id, glm::mat4 new_world_transform) {

}