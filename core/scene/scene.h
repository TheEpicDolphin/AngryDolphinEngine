#pragma once

#include "scene_graph.h"
#include <core/ecs/ecs.h>
#include <core/serialize/archive.h>
#include <core/serialize/serializable.h>
#include <core/graphics/renderer.h>

class Scene : ISerializable
{
public:

	void DidLoad();

	void DidUnload();

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

private:
	SceneGraph scene_graph_;
	ECS ecs_;
	IRenderer *renderer_;
};