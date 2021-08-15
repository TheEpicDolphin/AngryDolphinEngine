#pragma once

#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>
#include <GL/glew.h>

#include "camera.h"
#include "mesh_renderable.h"
#include "renderer.h"

class RenderingSystem : public System<RenderingSystem>
{
public:
	RenderingSystem() = default;

	RenderingSystem(ECS *ecs) : System<RenderingSystem>(ecs) {
		
	}

	void Update() 
	{
		std::vector<RenderableObjectInfo> renderable_objects;
		std::function<void(EntityID, MeshRenderable&, Transform&)> block =
		[&](EntityID entity_id, MeshRenderable& mesh_rend, Transform& trans) {
			renderable_objects.push_back({ mesh_rend.shared_mesh, trans.matrix });
		};
		ecs_->EnumerateComponentsWithBlock<MeshRenderable, Transform>(block);

		renderer_->RenderFrame(renderable_objects);
	}

private:
	IRenderer *renderer_;
};