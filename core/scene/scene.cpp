
#include "scene.h"

void DidLoad() 
{

}

void DidUnload() 
{

}

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

void Scene::SerializeHumanReadable(Archive& archive, std::ostream& ostream)
{
	archive.SerializeHumanReadable(
		ostream,
		std::make_pair("Scene_Graph", scene_graph_), 
		std::make_pair("ECS", ecs_ ), 
		std::make_pair("IRenderer", renderer_ )
	);
}

void Scene::DeserializeHumanReadable(Archive& archive, std::ostream& ostream)
{
	archive.DeserializeHumanReadable(
		ostream,
		std::make_pair("Scene_Graph", scene_graph_),
		std::make_pair("ECS", ecs_),
		std::make_pair("IRenderer", renderer_)
	);
}