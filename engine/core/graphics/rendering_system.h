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
private:
	IRenderer renderer_;

public:
	RenderingSystem() = default;

	RenderingSystem(ECS *ecs) : System<RenderingSystem>(ecs) {
		
	}

	void Update() 
	{
		std::function<void(EntityID, MeshRenderable&, Transform&)> block =
		[&](EntityID entity_id, MeshRenderable& mesh_rend, Transform& trans) {
			renderer_.AddRenderableObject(entity_id, { mesh_rend.material, mesh_rend.mesh, trans.matrix });
		};
		ecs_->EnumerateComponentsWithBlock<MeshRenderable, Transform>(block);

		std::function<void(EntityID, Camera&)> camera_block =
			[&](EntityID entity_id, MeshRenderable& mesh_rend, Transform& trans) {
			renderer_.SetRenderTarget(entity_id, {});
		};
		ecs_->EnumerateComponentsWithBlock<Camera>(camera_block);
	}
};