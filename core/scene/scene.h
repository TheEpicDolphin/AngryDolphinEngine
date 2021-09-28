#pragma once

#include "scene_graph.h"
#include <core/ecs/ecs.h>
#include <core/serialize/archive.h>

class Scene 
{
public:

	void Load();

	void Unload();

	EntityID CreateEntity();

	void DestroyEntity(EntityID entity_id);

	void SerializeHumanReadable(Archive& archive);

	void DeserializeHumanReadable(Archive& archive);

private:
	SceneGraph scene_graph_;
	ECS ecs_;
	IRenderer renderer_;
};