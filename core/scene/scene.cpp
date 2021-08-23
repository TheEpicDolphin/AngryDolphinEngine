
#include "scene.h"

EntityID Scene::CreateEntity() 
{
	const EntityID& entity_id = ecs_.CreateEntity();
	scene_graph_.CreateTransform(entity_id);
	return entity_id;
}

void Scene::DestroyEntity(EntityID entity_id)
{
	scene_graph_.DestroyTransform(entity_id);
	ecs_.DestroyEntity(entity_id);
}