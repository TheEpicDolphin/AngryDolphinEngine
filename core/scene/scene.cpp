
#include "scene.h"

EntityID IScene::CreateEntity() 
{
	const EntityID& entity_id = scene_graph_.CreateEntity();
	registry_.RegisterEntity(entity_id);
	return entity_id;
}

void IScene::DestroyEntity(EntityID entity_id)
{
	registry_.UnregisterEntity(entity_id);
	scene_graph_.DestroyEntity(entity_id);
}

// ISerializable

std::vector<ArchiveSerNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override {
	return archive.RegisterObjectForSerialization<SceneGraph&, ecs::Registry&, IRenderer*&>(
		{ "Scene_Graph", scene_graph_ },
		{ "ECS_Registry", registry_ },
		{ "Renderer", renderer_ },
	);
}

// IDeserializable

std::vector<ArchiveDesNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
{
	return archive.RegisterObjectsForDeserialization<SceneGraph&, ecs::Registry&, IRenderer*&>(
		xml_node,
		{ "Scene_Graph", scene_graph_ },
		{ "ECS_Registry", registry_ },
		{ "Renderer", renderer_ },
	);
}