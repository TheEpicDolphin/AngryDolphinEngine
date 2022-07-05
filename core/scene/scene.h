#pragma once

#include <core/ecs/registry.h>
#include <core/definitions/scene/scene_entity_instantiator.h>
#include <core/serialize/serializable.h>
#include <core/services/service_container.h>

#include "scene_graph.h"

class IScene {
public:
	virtual const char* Name() = 0;

	// TODO: OnLoadAsync and OnUnloadAsync

	virtual void OnLoad(ServiceContainer& service_container) = 0;

	virtual void OnUnload(ServiceContainer& service_container) = 0;

	virtual void OnFixedUpdate(double fixed_delta_time) = 0;

	virtual void OnFrameUpdate(double delta_time, double alpha) = 0;
};

class SceneBase : public IScene, public ISceneEntityInstantiator
{
public:
	SceneBase() = delete;

	SceneBase(const char* name) : name_(name) {}

	const char* Name() override { return name_; }

	void OnLoad(ServiceContainer& service_container) override {
		service_container.BindTo<ISceneEntityInstantiator>(*this);
		service_container.BindTo<SceneGraph>(scene_graph_);
		service_container.BindTo<ecs::Registry>(registry_);
	}

	void OnUnload(ServiceContainer& service_container) override {
		service_container.Unbind<ISceneEntityInstantiator>();
		service_container.Unbind<SceneGraph>();
		service_container.Unbind<ecs::Registry>();
	}

	void OnFixedUpdate(double fixed_delta_time) override {}

	void OnFrameUpdate(double delta_time, double alpha) override {}

	ecs::EntityID CreateEntity() override {
		ecs::EntityID entity_id = scene_graph_.CreateEntity();
		registry_.RegisterEntity(entity_id);
		return entity_id;
	}

	void DestroyEntity(ecs::EntityID entity_id) override {
		registry_.UnregisterEntity(entity_id);
		scene_graph_.DestroyEntity(entity_id);
	}

private:
	const char* name_;
	SceneGraph scene_graph_;
	ecs::Registry registry_;
};