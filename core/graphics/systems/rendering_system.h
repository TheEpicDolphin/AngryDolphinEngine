#pragma once

#include <core/ecs/system.h>
#include <core/scene/scene.h>
#include <core/scene/scene_graph.h>
#include <core/definitions/transform/transform_service.h>
#include <core/definitions/graphics/renderer.h>

class RenderingSystem : public ISystem
{
public:
	RenderingSystem() = default;

	void Initialize(ServiceContainer service_container) override;

	void OnInstantiateEntity(ecs::EntityID entity_id) {};

	void OnCleanupEntity(ecs::EntityID entity_id) {};

	void OnFixedUpdate(double fixed_delta_time) {}

	void OnFrameUpdate(double delta_time, double alpha) override;

private:
	ecs::Registry* component_registry_;
	ITransformService* transform_service_;
	IRenderer* renderer_;
	std::vector<RenderableObject> renderable_objects_;
};