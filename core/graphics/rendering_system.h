#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <core/scene/scene.h>
#include <core/scene/scene_graph.h>

#include "camera_component.h"
#include "components/mesh_renderable_component.h"
#include "components/skeletal_mesh_renderable_component.h"
#include "renderer.h"

class RenderingSystem : public ISystem
{
public:
	RenderingSystem() = default;

	void OnFrameUpdate(double delta_time, double alpha, const IScene& scene)
	{
		std::vector<RenderableObject> renderable_objects;
		// Iterate through mesh renderables.
		std::function<void(ecs::EntityID, MeshRenderableComponent&)> mesh_renderables_block =
		[&renderable_objects, &scene](ecs::EntityID entity_id, MeshRenderableComponent& mesh_rend) {
			if (mesh_rend.enabled) {
				
				const Mesh* mesh = mesh_rend.mesh ? mesh_rend.mesh.get() : mesh_rend.shared_mesh.get();
				const Material* material = mesh_rend.material ? mesh_rend.material.get() : mesh_rend.shared_material.get();
				assert(mesh->GetPipeline().InstanceID() == material->GetPipeline().InstanceID());
				renderable_objects.push_back({ mesh, , material, scene.TransformGraph().GetWorldTransform(entity_id), {} });
			}
		};
		scene.Registry().EnumerateComponentsWithBlock<MeshRenderableComponent>(mesh_renderables_block);

		// Iterate through skeletal mesh renderables.
		/*
		std::function<void(EntityID, SkeletalMeshRenderable&)> skel_mesh_renderables_block =
		[&renderable_objects, &scene](EntityID entity_id, SkeletalMeshRenderable& skel_mesh_rend) {
			if (skel_mesh_rend.enabled) {
				std::vector<glm::mat4> bone_transforms(skel_mesh_rend.bones.size());
				for (std::size_t i = 0; i < bone_transforms.size(); i++) {
					bone_transforms[i] = scene.TransformGraph().GetWorldTransform(skel_mesh_rend.bones[i]);
				}
				const MeshID mesh_id = skel_mesh_rend.unique_mesh ? skel_mesh_rend.unique_mesh.get() : skel_mesh_rend.shared_mesh.get();
				renderable_objects.push_back({ mesh_id, scene.TransformGraph().GetWorldTransform(entity_id), bone_transforms });
			}
		};
		scene.Registry().EnumerateComponentsWithBlock<SkeletalMeshRenderable>(skel_mesh_renderables_block);
		*/

		std::function<void(ecs::EntityID, CameraComponent&)> cameras_block =
			[&cameras, &scene, &renderable_objects](ecs::EntityID entity_id, CameraComponent& camera_component) {
			if (camera_component.enabled) {
				const glm::mat4 view_matrix = scene.TransformGraph().GetWorldTransform(entity_id);
				glm::mat4 projection_matrix;
				if (camera_component.is_orthographic) {
					// Orthographic projection matrix
					projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);
				}
				else {
					// Perspective projection matrix
					// Projection matrix : 45° Field of View, width:height ratio, display range : 0.1 unit (z near) <-> 100 units (z far)
					projection_matrix = glm::perspective(
						glm::radians(camera_component.vertical_fov),
						camera_component.aspect_ratio,
						camera_component.near_clip_plane_z,
						camera_component.far_clip_plane_z
					);
				}

				std::vector<RenderableObject> non_culled_renderable_objects;
				
				// TODO: Perform frustum culling of renderables.


				CameraParams cam_params = { view_matrix * projection_matrix, };
				scene.Renderer().RenderFrame(cam_params, non_culled_renderable_objects);
			}
		};
		scene.Registry().EnumerateComponentsWithBlock<CameraComponent>(cameras_block);
	}

private:
	std::unordered_map<ecs::EntityID, glm::mat4> entity_transform_map_;
	std::unordered_map<MeshID, std::vector<ecs::EntityID>>
};