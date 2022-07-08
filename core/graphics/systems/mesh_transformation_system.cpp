
#include "mesh_transformation_system.h"

#include <glm/gtc/type_ptr.hpp>
#include <core/transform/transform.h>

#include "../rendering_pipeline.h"

void MeshTransformationSystem::Initialize(ServiceContainer service_container) {
	if (!service_container.TryGetService(component_registry_)) {
		// TODO: Throw error.
	}

	if (!service_container.TryGetService(scene_graph_)) {
		// TODO: Throw error.
	}
}

void MeshTransformationSystem::OnInstantiateEntity(ecs::EntityID entity_id) {
	MeshRenderableComponent mesh_rend;
	if (!component_registry_->GetComponent<MeshRenderableComponent>(entity_id, mesh_rend)) {
		return;
	}

	Mesh* mesh_handle = mesh_rend.mesh.get();
	if (mesh_handle) {
		AddEntityToMesh2EntitiesMapping(entity_id, mesh_handle);
		if (mesh_rend.mesh->GetVertexPositions().size() > 0) {
			RecalculateMeshBounds(entity_id, mesh_rend);
		}
	}

	entity_mesh_trans_state_map_[entity_id.index] = { mesh_handle, false };
}

void MeshTransformationSystem::OnCleanupEntity(ecs::EntityID entity_id) {
	MeshRenderableComponent _;
	if (!component_registry_->GetComponent<MeshRenderableComponent>(entity_id, _)) {
		return;
	}

	Mesh* mesh_handle = entity_mesh_trans_state_map_[entity_id.index].mesh_handle;
	entity_mesh_trans_state_map_.erase(entity_id.index);

	RemoveEntityFromMesh2EntitiesMapping(entity_id, mesh_handle);
}

void MeshTransformationSystem::OnFrameUpdate(double delta_time, double alpha)
{
	// Iterate through mesh renderables and calculate world mesh bounds
	std::function<void(ecs::EntityID, MeshRenderableComponent&)> mesh_renderables_block =
		[this](ecs::EntityID entity_id, MeshRenderableComponent& mesh_rend) {
		if (mesh_rend.enabled) {
			std::shared_ptr<Mesh> mesh = mesh_rend.mesh;
			Mesh* mesh_handle = mesh.get();

			std::unordered_map<ecs::EntityIndex, MeshTransformationState>::iterator mesh_trans_state_iter = entity_mesh_trans_state_map_.find(entity_id.index);
			if (mesh_trans_state_iter == entity_mesh_trans_state_map_.end()) {
				// This should not happen. Throw error.
				return;
			}

			Mesh* previous_mesh_handle = mesh_trans_state_iter->second.mesh_handle;
			if (mesh_handle != previous_mesh_handle) {
				// This entity has changed meshes since the last update

				// Remove entity from previous mesh -> entities mapping.
				RemoveEntityFromMesh2EntitiesMapping(entity_id, previous_mesh_handle);

				if (mesh_handle) {
					// Add entity to new mesh -> entities mapping.
					AddEntityToMesh2EntitiesMapping(entity_id, mesh_handle);

					// Calculate the mesh bounds in world space
					if (mesh_rend.mesh->GetVertexPositions().size() > 0) {
						RecalculateMeshBounds(entity_id, mesh_rend);
					}
				}
			}

			entity_mesh_trans_state_map_[entity_id.index] = { mesh_handle, false };
		}
	};

	component_registry_->EnumerateComponentsWithBlock<MeshRenderableComponent>(mesh_renderables_block);
}

inline bool DidMeshVertexPositionsChange(Mesh* mesh, std::size_t attribute_index) {
	return mesh->GetPipeline()->VertexAttributeInfoAtIndex(attribute_index).category == VertexAttributeUsageCategory::Position;
}

#pragma region MeshLifecycleEventsListener

void MeshTransformationSystem::MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) {
	if (DidMeshVertexPositionsChange(mesh, attribute_index)) {
		// Mesh vertex position(s) were changed.
		std::vector<ecs::EntityID>& entities = mesh_to_entities_map_[mesh];
		for (ecs::EntityID entity : entities) {
			entity_mesh_trans_state_map_[entity.index].is_stale = true;
		}
	}
}

void MeshTransformationSystem::MeshDidDestroy(Mesh* mesh) {
	// no-op
}

#pragma endregion

#pragma region EntityLifecycleEventsListener

void MeshTransformationSystem::EntityWorldTransformDidChange(ecs::EntityID entity_id, glm::mat4 new_world_transform) {
	entity_mesh_trans_state_map_[entity_id.index].is_stale = true;
}

#pragma endregion

#pragma region Helpers

void MeshTransformationSystem::AddEntityToMesh2EntitiesMapping(ecs::EntityID entity_id, Mesh* mesh_handle) {
	std::unordered_map<Mesh*, std::vector<ecs::EntityID>>::iterator mesh_entities_iter = mesh_to_entities_map_.find(mesh_handle);
	if (mesh_entities_iter != mesh_to_entities_map_.end()) {
		mesh_entities_iter->second.push_back(entity_id);
	}
	else {
		mesh_to_entities_map_[mesh_handle] = { entity_id };
		mesh_handle->AddLifecycleEventsListener(this);
	}
}

#pragma endregion

void MeshTransformationSystem::RemoveEntityFromMesh2EntitiesMapping(ecs::EntityID entity_id, Mesh* mesh_handle) {
	auto mesh_to_entities_iter = mesh_to_entities_map_.find(mesh_handle);
	if (mesh_to_entities_iter != mesh_to_entities_map_.end()) {
		std::vector<ecs::EntityID>& entities_with_same_mesh = mesh_to_entities_iter->second;

		// Erase entity_id from vector
		entities_with_same_mesh.erase(
			std::remove(entities_with_same_mesh.begin(), entities_with_same_mesh.end(), entity_id),
			entities_with_same_mesh.end()
		);

		if (entities_with_same_mesh.empty()) {
			// No more entities with this mesh. Erase the mesh key.
			mesh_to_entities_map_.erase(mesh_handle);
			mesh_handle->RemoveLifecycleEventsListener(this);
		}
	}
}

void MeshTransformationSystem::RecalculateMeshBounds(ecs::EntityID entity_id, MeshRenderableComponent& mesh_rend) {
	const std::vector<glm::vec3>& local_vert_positions = mesh_rend.mesh->GetVertexPositions();
	glm::mat4 entity_transform = scene_graph_->GetWorldTransform(entity_id);
	glm::vec3 min_p = transform::TransformPointLocalToWorld(entity_transform, local_vert_positions[0]);
	glm::vec3 max_p = min_p;
	for (std::size_t i = 1; i < local_vert_positions.size(); i++) {
		glm::vec3 world_p = transform::TransformPointLocalToWorld(entity_transform, local_vert_positions[i]);
		min_p = glm::min(min_p, world_p);
		max_p = glm::max(max_p, world_p);
	}

	mesh_rend.world_mesh_bounds_ = geometry::Bounds(min_p, max_p);
}