#pragma once

#include "scene_graph.h"
#include <core/ecs/ecs.h>

class Scene 
{
public:

	void Load();

	void Unload();

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

private:
	SceneGraph scene_graph_;
	ECS ecs_;
	IRenderer renderer_;
};