#pragma once

#include "scene_graph.h"
#include <core/ecs/registry.h>
#include <core/serialize/serializable.h>
#include <core/graphics/renderer.h>

class Scene : ISerializable
{
public:

	void DidLoad();

	void DidUnload();

	void OnFixedUpdate(double fixed_delta_time);

	void OnUpdate(double delta_time, double alpha);

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

private:
	SceneGraph scene_graph_;
	ECS::Registry registry_;
	IRenderer *renderer_;

	PhysicsSystem physics_system_;
	PhysicsInterpolater physics_interpolator_;

	RenderingSystem rendering_system_;
	PhysicsSystem physics_system_;
	PhysicsInterpolationSystem physics_interp_system_;
};