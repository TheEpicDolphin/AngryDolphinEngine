#pragma once

#include <core/graphics/systems/rendering_system.h>
#include <core/graphics/systems/mesh_transformation_system.h>
#include <core/simulation/rigidbody_system.h>

#include "scene.h"

class SimpleScene : public SceneBase {
public:
	SimpleScene(const char* name, IRenderer* renderer) : SceneBase(name, renderer) {}

	void OnLoad() override {
		// Instantiate entities.

	}

	void OnUnload() override {}

	ecs::EntityID CreateEntity() override {
		ecs::EntityID entity_id = SceneBase::CreateEntity();

		mesh_transformation_system_.OnInstantiateEntity(entity_id);
		rigidbody_system_.OnInstantiateEntity(entity_id);
		rendering_system_.OnInstantiateEntity(entity_id);

		return entity_id;
	}

	void DestroyEntity(ecs::EntityID entity_id) override {
		mesh_transformation_system_.OnCleanupEntity(entity_id);
		rigidbody_system_.OnCleanupEntity(entity_id);
		rendering_system_.OnCleanupEntity(entity_id);

		SceneBase::DestroyEntity(entity_id);
	}

	void OnFixedUpdate(double fixed_delta_time) override 
	{
		rigidbody_system_.OnFixedUpdate(fixed_delta_time, *this);
	}

	void OnFrameUpdate(double delta_time, double alpha) override 
	{
		mesh_transformation_system_.OnFrameUpdate(delta_time, alpha, *this);

		// interpolate physics states to avoid jitter in render
		rigidbody_system_.OnFrameUpdate(delta_time, alpha, *this);

		// render
		rendering_system_.OnFrameUpdate(delta_time, alpha, *this);
	}

	// TODO: Fix below
	/*
	// ISerializable

	std::vector<ArchiveSerNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) override {
		std::vector<ArchiveSerNodeBase*> member_ser_nodes = IScene::RegisterMemberVariablesForSerialization(archive);
		std::vector<ArchiveSerNodeBase*> derived_member_ser_nodes =
		archive.RegisterObjectForSerialization<RenderingSystem&, RigidbodySystem&>(
			{ "Rendering_System", rendering_system_ },
			{ "Rigidbody_System", rigidbody_system_ }
		);
		member_ser_nodes.insert(member_ser_nodes.end(), derived_member_ser_nodes.begin(), derived_member_ser_nodes.end());
		return member_ser_nodes;
	}

	// IDeserializable

	std::vector<ArchiveDesNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		std::vector<ArchiveDesNodeBase*> member_des_nodes = IScene::RegisterMemberVariablesForDeserialization(archive, xml_node);
		std::vector<ArchiveDesNodeBase*> derived_member_des_nodes =
		archive.RegisterObjectsForDeserialization<RenderingSystem&, RigidbodySystem&>(
			xml_node,
			{ "Rendering_System", rendering_system_ },
			{ "Rigidbody_System", rigidbody_system_ }
		);
		member_des_nodes.insert(member_des_nodes.end(), derived_member_des_nodes.begin(), derived_member_des_nodes.end());
		return member_des_nodes;
	}
	*/

private:
	MeshTransformationSystem mesh_transformation_system_;
	RenderingSystem rendering_system_;
	RigidbodySystem rigidbody_system_;
	// TODO: Animation System, Collider System, etc.
};