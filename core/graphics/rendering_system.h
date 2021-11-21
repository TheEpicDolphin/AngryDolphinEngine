#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/registry.h>
#include <core/scene/scene_graph.h>
#include <GL/glew.h>

#include "camera.h"
#include "mesh_renderable.h"
#include "renderer.h"

class RenderingSystem : public IFrameUpdateSystem
{
public:
	RenderingSystem() = default;

	void OnFrameUpdate(double delta_time, double alpha, const Scene& scene)
	{
		std::vector<RenderableObjectInfo> renderable_objects;
		std::function<void(EntityID, MeshRenderable&)> block =
		[&](EntityID entity_id, MeshRenderable& mesh_rend) {
			if (mesh_rend.enabled) {
				renderable_objects.push_back({ mesh_rend.shared_mesh, scene.TransformGraph().GetWorldTransform(entity_id) });
			}
		};
		scene.Registry().EnumerateComponentsWithBlock<MeshRenderable>(block);
		scene.Renderer().RenderFrame(renderable_objects);
	}
};