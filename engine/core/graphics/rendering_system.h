#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>
#include <core/transform/transform.h>

#include "mesh_renderer.h"

class RenderingSystem : public System<RenderingSystem>
{
public:
	RenderingSystem(ECS ecs) : System<RenderingSystem>(ecs) {

	}

	void Update() 
	{
		std::function<void(EntityID, MeshRenderer&, Transform&)> block =
		[](EntityID entity_id, MeshRenderer& mesh_rend, Transform& trans) {
			// TODO: render each entity
		};
		ecs_.EnumerateComponentsWithBlock<MeshRenderer, Transform>(block);
	}
};