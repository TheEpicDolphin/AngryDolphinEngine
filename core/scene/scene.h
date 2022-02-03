#pragma once

#include <core/ecs/registry.h>
#include <core/graphics/renderer.h>
#include <core/serialize/serializable.h>

#include "interfaces/transform_graph.h"
#include "scene_graph.h"

class IScene {
	virtual void DidLoad() = 0;

	virtual void DidUnload() = 0;

	virtual void OnFixedUpdate(double fixed_delta_time) = 0;

	virtual void OnFrameUpdate(double delta_time, double alpha) = 0;

	virtual ecs::EntityID CreateEntity() = 0;

	virtual void DestroyEntity(ecs::EntityID entity_id) = 0;

	virtual const ITransformGraph& TransformGraph() = 0;

	virtual const ecs::Registry& ComponentRegistry() = 0;

	virtual const IRenderer& Renderer() = 0;
};

class SceneBase : public IScene, public ISerializable, public IDeserializable
{
public:
	void DidLoad() override {};

	void DidUnload() override {};

	void OnFixedUpdate(double fixed_delta_time) override {};

	void OnFrameUpdate(double delta_time, double alpha) override {};

	ecs::EntityID CreateEntity() override {
		ecs::EntityID entity_id = scene_graph_.CreateEntity();
		registry_.RegisterEntity(entity_id);
	}

	void DestroyEntity(ecs::EntityID entity_id) override {
		registry_.UnregisterEntity(entity_id);
		scene_graph_.DestroyEntity(entity_id);
	}

	const ITransformGraph& TransformGraph() override {
		return scene_graph_;
	}

	const ecs::Registry& ComponentRegistry() override {
		return registry_;
	}

	const IRenderer& Renderer() override {
		return renderer_;
	}

private:
	SceneGraph scene_graph_;
	ecs::Registry registry_;
	IRenderer *renderer_;
};