
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

void Scene::SerializeHumanReadable(Archive& archive)
{
	archive.SerializeHumanReadable(
		std::make_pair("Scene Graph", scene_graph_), 
		std::make_pair("ECS", ecs_ ), 
		std::make_pair("IRenderer", renderer_ )
	);
}

void Scene::DeserializeHumanReadable(Archive& archive)
{
	archive.DeserializeHumanReadable(
		std::make_pair("Scene Graph", scene_graph_),
		std::make_pair("ECS", ecs_),
		std::make_pair("IRenderer", renderer_)
	);
}