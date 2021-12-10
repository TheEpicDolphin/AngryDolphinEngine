#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <core/scene/scene.h>
#include <core/scene/scene_graph.h>
#include <GL/glew.h>

#include "camera.h"
#include "mesh_renderable.h"
#include "skeletal_mesh_renderable.h"
#include "renderer.h"

class RenderingSystem : public ISystem
{
public:
	RenderingSystem() = default;

	void OnFrameUpdate(double delta_time, double alpha, const IScene& scene)
	{
		std::vector<RenderableObject> renderable_objects;
		// Iterate through mesh renderables.
		std::function<void(EntityID, MeshRenderable&)> mesh_renderables_block =
		[&renderable_objects, &scene](EntityID entity_id, MeshRenderable& mesh_rend) {
			if (mesh_rend.enabled) {
				const MeshID mesh_id = mesh_rend.unique_mesh ? mesh_rend.unique_mesh.get() : mesh_rend.shared_mesh.get();
				renderable_objects.push_back({ mesh_id, scene.TransformGraph().GetWorldTransform(entity_id), {} });
			}
		};
		scene.Registry().EnumerateComponentsWithBlock<MeshRenderable>(mesh_renderables_block);

		// Iterate through skeletal mesh renderables.
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

		scene.Renderer().RenderFrame(renderable_objects);
	}
};