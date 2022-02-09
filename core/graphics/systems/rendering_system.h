#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <core/geometry/bounds.h>
#include <core/scene/scene.h>
#include <core/scene/scene_graph.h>
#include <core/scene/interfaces/transform_graph.h>
#include <core/transform/transform.h>

#include "../components/camera_component.h"
#include "../components/mesh_renderable_component.h"
#include "../components/skeletal_mesh_renderable_component.h"
#include "../renderer.h"

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
				Mesh* mesh = mesh_rend.mesh ? mesh_rend.mesh.get() : mesh_rend.shared_mesh.get();
				Material* material = mesh_rend.material ? mesh_rend.material.get() : mesh_rend.shared_material.get();
				assert(mesh->GetPipeline()->InstanceID() == material->GetPipeline()->InstanceID());
				renderable_objects.push_back({ 
					mesh, 
					material, 
					scene.TransformGraph().GetWorldTransform(entity_id), 
					mesh_rend.WorldMeshBounds(), 
					{} 
				});
			}
		};
		scene.ComponentRegistry().EnumerateComponentsWithBlock<MeshRenderableComponent>(mesh_renderables_block);

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
			[&scene, &renderable_objects](ecs::EntityID entity_id, CameraComponent& camera_component) {
			if (camera_component.enabled) {
				const glm::mat4 view_matrix = scene.TransformGraph().GetWorldTransform(entity_id);

				std::vector<RenderableObject> non_culled_renderable_objects;
				glm::mat4 projection_matrix;
				if (camera_component.is_orthographic) {
					// Orthographic projection matrix
					projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);
				}
				else {
					float vertical_fov_radians = glm::radians(camera_component.vertical_fov);
					// Perspective projection matrix
					// Projection matrix : 45° Field of View, width:height ratio, display range : 0.1 unit (z near) <-> 100 units (z far)
					projection_matrix = glm::perspective(
						vertical_fov_radians,
						camera_component.aspect_ratio,
						camera_component.near_clip_plane_z,
						camera_component.far_clip_plane_z
					);

					const float hh_1 = tan(vertical_fov_radians / 2);
					const float hh_near = hh_1 * camera_component.near_clip_plane_z;
					const float hw_near = hh_near * camera_component.aspect_ratio;
					const float hh_far = hh_1 * hh_near * camera_component.far_clip_plane_z;
					const float hw_far = hh_far * camera_component.aspect_ratio;
					
					// Bounds of the view frustum when transformed to clip space.
					geometry::Bounds view_frustum_clip_space_bounds = geometry::Bounds(
						glm::vec3(-hw_near, -hh_near, camera_component.near_clip_plane_z), 
						glm::vec3(hw_near, hh_near, camera_component.far_clip_plane_z)
					);

					// For each renderable object, check if any of the AABB points are in the view frustum 
					// after transformed into clip space.
					for (RenderableObject renderable_object : renderable_objects) {
						const geometry::Bounds& aabb = renderable_object.aabb;
						const glm::vec3 aabb_points[8] = {
							aabb.min,
							glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
							glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
							glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
							glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
							aabb.max,
							glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
							glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
						};

						for (std::size_t i = 0; i < 8; i++) {
							glm::vec3 aabb_point_clip = transform::TransformPointWorldToLocal(projection_matrix, aabb_points[i]);
							if (view_frustum_clip_space_bounds.ContainsPoint(aabb_point_clip)) {
								non_culled_renderable_objects.push_back(renderable_object);
								break;
							}
						}
					}

					glm::vec3 camera_forward = transform::ForwardVectorFromTransform(view_matrix);
					glm::vec3 camera_up = transform::UpVectorFromTransform(view_matrix);
					glm::vec3 view_frustum_corners_world[8] = {
						// TODO
					};

					// For each renderable object, check if any of the view frustum corners (in world space) 
					// are within its AABB bounds. This catches edge cases where the renderable object AABBs
					// do not have any points within the view frustum, but still intersect.
					for (RenderableObject renderable_object : renderable_objects) {
						for (std::size_t i = 0; i < 8; i++) {
							if (renderable_object.aabb.ContainsPoint(view_frustum_corners_world[i])) {
								non_culled_renderable_objects.push_back(renderable_object);
							}
						}
					}
				}

				CameraParams cam_params = { view_matrix * projection_matrix, camera_component.viewport_rect };
				scene.Renderer().RenderFrame(cam_params, non_culled_renderable_objects);
			}
		};
		scene.ComponentRegistry().EnumerateComponentsWithBlock<CameraComponent>(cameras_block);
	}
};