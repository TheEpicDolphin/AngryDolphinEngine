#pragma once

#include <core/ecs/registry.h>
#include <core/ecs/system.h>
#include <core/serialize/serializable.h>
#include <core/graphics/renderer.h>

#include "scene_graph.h"

class Scene : ISerializable
{
public:

	Scene();

	void DidLoad();

	void DidUnload();

	void OnFixedUpdate(double fixed_delta_time);

	void OnFrameUpdate(double delta_time, double alpha);

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

	//void RegisterFixedUpdateSystem(IFixedUpdateSystem* system, SystemOrder order);

	//void RegisterFrameUpdateSystem(IFrameUpdateSystem* system, SystemOrder order);

	const ITransformGraph& TransformGraph() {
		return scene_graph_;
	}

	const ecs::Registry& Registry() {
		return registry_;
	}

	const IRenderer& Renderer() {
		return renderer_;
	}

private:
	SceneGraph scene_graph_;
	ecs::Registry registry_;
	IRenderer *renderer_;

	RenderingSystem rendering_system_;
	PhysicsSystem physics_system_;

	//std::map<> fixedUpdateSystems;
	//std::map<> frameUpdateSystems;
};