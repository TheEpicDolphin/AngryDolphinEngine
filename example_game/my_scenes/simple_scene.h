#pragma once

#include <core/definitions/transform/transform_service.h>
#include <core/graphics/components/camera_component.h>
#include <core/graphics/components/mesh_renderable_component.h>
#include <core/graphics/systems/rendering_system.h>
#include <core/graphics/systems/mesh_transformation_system.h>
#include <core/scene/scene.h>
#include <core/simulation/rigidbody_system.h>
#include <core/transform/transform.h>

class SimpleScene : public SceneBase {
public:
	SimpleScene(const char* name) : SceneBase(name) {}

	void OnLoad(ServiceContainer& service_container) override {
		SceneBase::OnLoad(service_container);

		// Bind services.
		service_container.BindTo<ISceneEntityInstantiator>(*this);

		mesh_transformation_system_.Initialize(service_container);
		rigidbody_system_.Initialize(service_container);
		rendering_system_.Initialize(service_container);

		// Instantiate entities.
		ecs::Registry* component_registry;
		if (!service_container.TryGetService(component_registry)) {
			return;
		}

		ITransformService* transform_service;
		if (!service_container.TryGetService(transform_service)) {
			return;
		}

		std::cout << "setting up camera..." << std::endl;
		// Setup camera
		ecs::EntityID camera_entity = CreateEntity();
		CameraComponent camera_component;
		camera_component.disabled = false;
		camera_component.is_orthographic = true;
		camera_component.orthographic_half_height = 10.0f;
		camera_component.aspect_ratio = 4.0f / 3.0f;
		camera_component.near_clip_plane_z = 0.1f;
		camera_component.far_clip_plane_z = 100.0f;
		std::cout << "adding component..." << std::endl;
		component_registry->AddComponent<CameraComponent>(camera_entity, camera_component);
		std::cout << "added component" << std::endl;
		glm::mat4 camera_transform = glm::mat4(1.0f);
		transform::SetPosition(camera_transform, glm::vec3(0, 0, -5));
		transform_service->SetWorldTransform(camera_entity, camera_transform);

		std::cout << "setting up box..." << std::endl;
		// Setup box
		ecs::EntityID box_entity = CreateEntity();
		MeshRenderableComponent mesh_rend_component;
		mesh_rend_component.disabled = false;
		std::shared_ptr<RenderingPipeline> rp = RenderingPipeline::RenderingPipelineForResourcePath("standard_rendering_pipeline/standard.xml");
		mesh_rend_component.mesh = Mesh::CreateCubeMeshPrimitive({ rp, false }, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
		mesh_rend_component.material = Material::CreateMaterial({ rp });
		component_registry->AddComponent<MeshRenderableComponent>(box_entity, mesh_rend_component);
		transform_service->SetWorldTransform(box_entity, glm::mat4(1.0));
	}

	void OnUnload(ServiceContainer& service_container) override {
		// Unbind services
		service_container.Unbind<ISceneEntityInstantiator>();

		// Delete entities.

		SceneBase::OnUnload(service_container);
	}

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
		rigidbody_system_.OnFixedUpdate(fixed_delta_time);
	}

	void OnFrameUpdate(double delta_time, double alpha) override 
	{
		mesh_transformation_system_.OnFrameUpdate(delta_time, alpha);

		// interpolate physics states to avoid jitter in render
		rigidbody_system_.OnFrameUpdate(delta_time, alpha);

		// render
		rendering_system_.OnFrameUpdate(delta_time, alpha);
	}

private:
	MeshTransformationSystem mesh_transformation_system_;
	RenderingSystem rendering_system_;
	RigidbodySystem rigidbody_system_;
	// TODO: Animation System, Collider System, etc.
};