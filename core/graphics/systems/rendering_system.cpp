
#include "rendering_system.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <core/definitions/graphics/renderer.h>
#include <core/ecs/registry.h>
#include <core/geometry/bounds.h>
#include <core/geometry/line2d.h>
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
			std::function<bool(geometry::Bounds&)> intersects_view;
			if (camera_component.is_orthographic) {
				// Bounds of the view frustum when transformed to clip space.
				const float orthographic_half_width = camera_component.aspect_ratio * camera_component.orthographic_half_height;
				const float orthographic_half_height = camera_component.orthographic_half_height;

				projection_matrix = glm::ortho(
					-orthographic_half_width,
					orthographic_half_width,
					-orthographic_half_height,
					orthographic_half_height,
					camera_component.near_clip_plane_z,
					camera_component.far_clip_plane_z
				);

				// Bounds of the ortho camera view, in camera space.
				geometry::Bounds ortho_view_bounds = geometry::Bounds(
					glm::vec3(-orthographic_half_width, -orthographic_half_height, -camera_component.far_clip_plane_z),
					glm::vec3(orthographic_half_width, orthographic_half_height, -camera_component.near_clip_plane_z)
				);

				intersects_view = [&ortho_view_bounds](geometry::Bounds& renderable_camera_space_aabb) {
					return ortho_view_bounds.Intersects(renderable_camera_space_aabb);
				};
			}
			else {
				const float vertical_fov_radians = glm::radians(camera_component.vertical_fov);
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

				const float max_z_bound = -camera_component.near_clip_plane_z;
				const float min_z_bound = -camera_component.far_clip_plane_z;
				geometry::Line2D min_xz_bound(
					glm::vec2(-hw_far, min_z_bound),
					glm::vec2(-hw_near, max_z_bound)
				);
				geometry::Line2D max_xz_bound(
					glm::vec2(hw_far, min_z_bound),
					glm::vec2(hw_near, max_z_bound)
				);
				geometry::Line2D min_yz_bound(
					glm::vec2(-hh_far, min_z_bound),
					glm::vec2(-hh_near, max_z_bound)
				);
				geometry::Line2D max_yz_bound(
					glm::vec2(hh_far, min_z_bound),
					glm::vec2(hh_near, max_z_bound)
				);
				intersects_view = [
					&min_xz_bound,
					&max_xz_bound,
					&min_yz_bound,
					&max_yz_bound,
					min_z_bound,
					max_z_bound
				](geometry::Bounds& aabb) {
					const glm::vec2 min_x_min_z = glm::vec2(aabb.min.x, aabb.min.z);
					const glm::vec2 max_x_min_z = glm::vec2(aabb.max.x, aabb.min.z);
					const glm::vec2 min_y_min_z = glm::vec2(aabb.min.y, aabb.min.z);
					const glm::vec2 max_y_min_z = glm::vec2(aabb.max.y, aabb.min.z);
					return !min_xz_bound.IsPointOnLeftSide(max_x_min_z) && max_xz_bound.IsPointOnLeftSide(min_x_min_z)
						&& !min_yz_bound.IsPointOnLeftSide(max_y_min_z) && max_yz_bound.IsPointOnLeftSide(min_y_min_z)
						&& min_z_bound <= aabb.max.z && max_z_bound >= aabb.min.z;
				};
			}

			// For each renderable object, transform its AABB points into camera space
			// and create a new bounding box (AABB') encapsulating its transformed points.
			// Then, perform an intersection test between AABB' and the view bounds/frustum
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

				glm::vec3 min_cam_space_p = transform::TransformedPoint(
					camera_view_matrix,
					renderable_aabb_points[0]
				);
				glm::vec3 max_cam_space_p = min_cam_space_p;
				for (std::size_t i = 1; i < 8; i++) {
					glm::vec3 renderable_cam_space_aabb_point = transform::TransformedPoint(
						camera_view_matrix,
						renderable_aabb_points[i]
					);
					min_cam_space_p = glm::min(min_cam_space_p, renderable_cam_space_aabb_point);
					max_cam_space_p = glm::max(max_cam_space_p, renderable_cam_space_aabb_point);
				}

				geometry::Bounds renderable_camera_space_aabb(min_cam_space_p, max_cam_space_p);
				if (intersects_view(renderable_camera_space_aabb)) {
					non_culled_renderable_objects_.push_back(renderable_object);
				}
			}

			CameraParams cam_params = { projection_matrix * camera_view_matrix, camera_component.viewport_rect };
			std::cout << "total renderables: " << renderable_objects_.size() << std::endl;
			std::cout << "culled: " << renderable_objects_.size() - non_culled_renderable_objects_.size() << std::endl;
			auto a = projection_matrix * camera_view_matrix;
			renderer_->RenderFrame(cam_params, non_culled_renderable_objects_);
		}
	};
	component_registry_->EnumerateComponentsWithBlock<CameraComponent>(cameras_block);
}