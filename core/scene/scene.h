#pragma once

#include <core/ecs/registry.h>
#include <core/graphics/renderer.h>
#include <core/serialize/serializable.h>

#include "interfaces/transform_graph.h"
#include "scene_graph.h"

class IScene : public ISerializable, public IDeserializable
{
public:
	virtual void DidLoad() = 0;

	virtual void DidUnload() = 0;

	virtual void OnFixedUpdate(double fixed_delta_time) = 0;

	virtual void OnFrameUpdate(double delta_time, double alpha) = 0;

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

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
};