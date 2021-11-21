
#include "scene.h"

Scene::Scene() {}

void Scene::DidLoad()
{

}

void Scene::DidUnload()
{

}

void Scene::OnFixedUpdate(double fixed_delta_time) {

}

void Scene::OnFrameUpdate(double delta_time, double alpha) {

}

EntityID Scene::CreateEntity() 
{
	const EntityID& entity_id = scene_graph_.CreateEntity();
	registry_.RegisterEntity(entity_id);
	return entity_id;
}

void Scene::DestroyEntity(EntityID entity_id)
{
	registry_.UnregisterEntity(entity_id);
	scene_graph_.DestroyEntity(entity_id);
}

void Scene::SerializeHumanReadable(Archive& archive, std::ostream& ostream)
{
	archive.SerializeHumanReadable(
		ostream,
		std::make_pair("Scene_Graph", scene_graph_), 
		std::make_pair("ECS", ecs_), 
		std::make_pair("IRenderer", renderer_)
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