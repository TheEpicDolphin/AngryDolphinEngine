
#include "rendering_system.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <core/definitions/graphics/renderer.h>
#include <core/ecs/registry.h>
#include <core/geometry/bounds.h>
#include <core/transform/transform.h>

#include "../components/camera_component.h"
#include "../components/mesh_renderable_component.h"
#include "../components/skeletal_mesh_renderable_component.h"

void RenderingSystem::Initialize(ServiceContainer service_container) {
	if (!service_container.TryGetService(component_registry_)) {
		// TODO: Throw error.
	}

	if (!service_container.TryGetService(transform_service_)) {
		// TODO: Throw error.
	}

	if (!service_container.TryGetService(renderer_)) {
		// TODO: Throw error.
	}
}

void RenderingSystem::Cleanup(ServiceContainer service_container) {}

void RenderingSystem::OnFrameUpdate(double delta_time, double alpha)
{
	renderable_objects_.clear();
	// Iterate through mesh renderables.
	std::function<void(ecs::EntityID, MeshRenderableComponent&)> mesh_renderables_block =
		[this](ecs::EntityID entity_id, MeshRenderableComponent& mesh_rend) {
		if (!mesh_rend.disabled) {
			assert(mesh_rend.mesh->GetPipeline() == mesh_rend.material->GetPipeline());
			renderable_objects_.push_back({
				mesh_rend.mesh.get(),
				mesh_rend.material.get(),
				transform_service_->GetWorldTransform(entity_id),
				mesh_rend.WorldMeshBounds(),
				{}
			});
		}
	};
	component_registry_->EnumerateComponentsWithBlock<MeshRenderableComponent>(mesh_renderables_block);

	non_culled_renderable_objects_.clear();
	std::function<void(ecs::EntityID, CameraComponent&)> cameras_block =
		[this](ecs::EntityID entity_id, CameraComponent& camera_component) {
		if (!camera_component.disabled) {
			const glm::mat4 camera_transform = transform_service_->GetWorldTransform(entity_id);
			const glm::mat4 camera_view_matrix = glm::inverse(camera_transform);

			glm::mat4 projection_matrix;
			geometry::Bounds view_frustum_clip_space_bounds;
			glm::vec3 view_frustum_corners_world[8];

			std::vector<RenderableObject> non_culled_renderable_objects;
			if (camera_component.is_orthographic) {
				// Orthographic projection matrix
				projection_matrix = glm::ortho(
					-10.0f,
					10.0f,
					-10.0f,
					10.0f,
					camera_component.near_clip_plane_z,
					camera_component.far_clip_plane_z
				);

				// Bounds of the view frustum when transformed to clip space.
				const float orthographic_half_width = camera_component.aspect_ratio * camera_component.orthographic_half_height;
				const float orthographic_half_height = camera_component.orthographic_half_height;
				view_frustum_clip_space_bounds = geometry::Bounds(
					glm::vec3(-orthographic_half_width, -orthographic_half_height, camera_component.near_clip_plane_z),
					glm::vec3(orthographic_half_width, orthographic_half_height, camera_component.far_clip_plane_z)
				);
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
				const float hh_far = hh_1 * camera_component.far_clip_plane_z;
				const float hw_far = hh_far * camera_component.aspect_ratio;

				// Bounds of the view frustum when transformed to clip space.
				view_frustum_clip_space_bounds = geometry::Bounds(
					glm::vec3(-hw_near, -hh_near, camera_component.near_clip_plane_z),
					glm::vec3(hw_near, hh_near, camera_component.far_clip_plane_z)
				);
			}

			const glm::mat4 camera_clip_space_matrix = camera_view_matrix * projection_matrix;

			// For each renderable object, transform its AABB points into camera clip space
			// and create a new bounding box (AABB') encapsulating its transformed points.
			// Then, perform an intersection test between AABB' and the view frustum clip
			// space AABB.

			// For each renderable object, check if any of the AABB points are in the view frustum 
			// after transformed into clip space.
			for (RenderableObject renderable_object : renderable_objects_) {
				const geometry::Bounds& aabb = renderable_object.aabb;
				const glm::vec3 renderable_aabb_points[8] = {
					aabb.min,
					glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
					glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
					glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
					glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
					aabb.max,
					glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
					glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
				};

				glm::vec3 min_clip_space_p = transform::TransformPointWorldToLocal(
					camera_clip_space_matrix,
					renderable_aabb_points[0]
				);
				glm::vec3 max_clip_space_p = min_clip_space_p;
				for (std::size_t i = 1; i < 8; i++) {
					glm::vec3 renderable_clip_space_aabb_point = transform::TransformPointWorldToLocal(
						camera_clip_space_matrix,
						renderable_aabb_points[i]
					);
					min_clip_space_p = glm::min(min_clip_space_p, renderable_clip_space_aabb_point);
					max_clip_space_p = glm::max(max_clip_space_p, renderable_clip_space_aabb_point);
				}

				geometry::Bounds renderable_clip_space_aabb(min_clip_space_p, max_clip_space_p);
				if (view_frustum_clip_space_bounds.Intersects(renderable_clip_space_aabb)) {
					non_culled_renderable_objects_.push_back(renderable_object);
					break;
				}
			}

			CameraParams cam_params = { camera_clip_space_matrix, camera_component.viewport_rect };
			std::cout << non_culled_renderable_objects.size() << std::endl;
			renderer_->RenderFrame(cam_params, non_culled_renderable_objects);
		}
	};
	/*
	std::function<void(ecs::EntityID, CameraComponent&)> cameras_block =
		[this](ecs::EntityID entity_id, CameraComponent& camera_component) {
		if (!camera_component.disabled) {
			const glm::mat4 camera_transform = transform_service_->GetWorldTransform(entity_id);
			const glm::mat4 camera_view_matrix = glm::inverse(camera_transform);

			glm::mat4 projection_matrix;
			geometry::Bounds view_frustum_clip_space_bounds;
			glm::vec3 view_frustum_corners_world[8];

			std::vector<RenderableObject> non_culled_renderable_objects;
			if (camera_component.is_orthographic) {
				// Orthographic projection matrix
				projection_matrix = glm::ortho(
					-10.0f,
					10.0f,
					-10.0f,
					10.0f,
					camera_component.near_clip_plane_z,
					camera_component.far_clip_plane_z
				);

				// Bounds of the view frustum when transformed to clip space.
				const float orthographic_half_width = camera_component.aspect_ratio * camera_component.orthographic_half_height;
				const float orthographic_half_height = camera_component.orthographic_half_height;
				view_frustum_clip_space_bounds = geometry::Bounds(
					glm::vec3(-orthographic_half_width, -orthographic_half_height, camera_component.near_clip_plane_z),
					glm::vec3(orthographic_half_width, orthographic_half_height, camera_component.far_clip_plane_z)
				);

				const glm::vec3 camera_left = transform::Left(camera_transform);
				const glm::vec3 camera_forward = transform::Forward(camera_transform);
				const glm::vec3 camera_up = transform::Up(camera_transform);
				const glm::vec3 camera_origin = transform::Position(camera_transform);

				std::cout << "origin: " << glm::to_string(camera_origin) << std::endl;
				std::cout << "forward: " << glm::to_string(camera_forward) << std::endl;
				std::cout << "left: " << glm::to_string(camera_left) << std::endl;
				std::cout << "up: " << glm::to_string(camera_up) << std::endl;

				const glm::vec3 near_clip_plane_center_world = camera_origin + camera_component.near_clip_plane_z * camera_forward;
				const glm::vec3 far_clip_plane_center_world = camera_origin + camera_component.far_clip_plane_z * camera_forward;
				const glm::vec3 hw_left = orthographic_half_width * camera_left;
				const glm::vec3 hh_up = orthographic_half_height * camera_up;

				view_frustum_corners_world[0] = near_clip_plane_center_world + hw_left + hh_up;
				view_frustum_corners_world[1] = near_clip_plane_center_world - hw_left + hh_up;
				view_frustum_corners_world[2] = near_clip_plane_center_world - hw_left - hh_up;
				view_frustum_corners_world[3] = near_clip_plane_center_world + hw_left - hh_up;
				view_frustum_corners_world[4] = far_clip_plane_center_world + hw_left + hh_up;
				view_frustum_corners_world[5] = far_clip_plane_center_world - hw_left + hh_up;
				view_frustum_corners_world[6] = far_clip_plane_center_world - hw_left - hh_up;
				view_frustum_corners_world[7] = far_clip_plane_center_world + hw_left - hh_up;
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
				const float hh_far = hh_1 * camera_component.far_clip_plane_z;
				const float hw_far = hh_far * camera_component.aspect_ratio;

				// Bounds of the view frustum when transformed to clip space.
				view_frustum_clip_space_bounds = geometry::Bounds(
					glm::vec3(-hw_near, -hh_near, camera_component.near_clip_plane_z),
					glm::vec3(hw_near, hh_near, camera_component.far_clip_plane_z)
				);

				const glm::vec3 camera_left = transform::Left(camera_transform);
				const glm::vec3 camera_forward = transform::Forward(camera_transform);
				const glm::vec3 camera_up = transform::Up(camera_transform);
				const glm::vec3 camera_origin = transform::Position(camera_transform);

				const glm::vec3 near_clip_plane_center_world = camera_origin + camera_component.near_clip_plane_z * camera_forward;
				const glm::vec3 hw_near_left = hw_near * camera_left;
				const glm::vec3 hh_near_up = hh_near * camera_up;

				const glm::vec3 far_clip_plane_center_world = camera_origin + camera_component.far_clip_plane_z * camera_forward;
				const glm::vec3 hw_far_left = hw_far * camera_left;
				const glm::vec3 hh_far_up = hh_far * camera_up;

				view_frustum_corners_world[0] = near_clip_plane_center_world + hw_near_left + hh_near_up;
				view_frustum_corners_world[1] = near_clip_plane_center_world - hw_near_left + hh_near_up;
				view_frustum_corners_world[2] = near_clip_plane_center_world - hw_near_left - hh_near_up;
				view_frustum_corners_world[3] = near_clip_plane_center_world + hw_near_left - hh_near_up;
				view_frustum_corners_world[4] = far_clip_plane_center_world + hw_far_left + hh_far_up;
				view_frustum_corners_world[5] = far_clip_plane_center_world - hw_far_left + hh_far_up;
				view_frustum_corners_world[6] = far_clip_plane_center_world - hw_far_left - hh_far_up;
				view_frustum_corners_world[7] = far_clip_plane_center_world + hw_far_left - hh_far_up;
			}

			const glm::mat4 camera_clip_space_matrix = camera_view_matrix * projection_matrix;

			// For each renderable object, check if any of the AABB points are in the view frustum 
			// after transformed into clip space.
			for (RenderableObject renderable_object : renderable_objects_) {
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
					glm::vec3 clip_space_aabb_point = transform::TransformPointWorldToLocal(camera_clip_space_matrix, aabb_points[i]);
					if (view_frustum_clip_space_bounds.ContainsPoint(clip_space_aabb_point)) {
						non_culled_renderable_objects.push_back(renderable_object);
						break;
					}
				}
			}

			// For each renderable object, check if any of the view frustum corners (in world space) 
			// are within its AABB bounds. This catches edge cases where the renderable object AABBs
			// do not have any points within the view frustum, but still intersect.
			for (RenderableObject renderable_object : renderable_objects_) {
				for (std::size_t i = 0; i < 8; i++) {
					if (renderable_object.aabb.ContainsPoint(view_frustum_corners_world[i])) {
						non_culled_renderable_objects.push_back(renderable_object);
						break;
					}
				}
			}

			CameraParams cam_params = { camera_clip_space_matrix, camera_component.viewport_rect };
			std::cout << non_culled_renderable_objects.size() << std::endl;
			renderer_->RenderFrame(cam_params, non_culled_renderable_objects);
		}
	};
	*/
	component_registry_->EnumerateComponentsWithBlock<CameraComponent>(cameras_block);
}