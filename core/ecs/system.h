#pragma once

class IScene;
class ServiceContainer;

// TODO: Extensions of this class will include more lifecycle methods such as WillRenderFrame, DidAnimate, etc.
// TODO: Create wrapper for IScene that prevents reassignment to avoid client reassigning scene reference in methods below.

class ISystem {
public:
	virtual void Initialize(ServiceContainer serviceContainer) = 0;

	virtual void OnInstantiateEntity(ecs::EntityID entity_id) = 0;

	virtual void OnCleanupEntity(ecs::EntityID entity_id) = 0;

	virtual void OnFixedUpdate(double fixed_delta_time, IScene& scene) = 0;

	virtual void OnFrameUpdate(double delta_time, double alpha, IScene& scene) = 0;
};

class SystemBase : public ISystem {
public:
	void Initialize(ServiceContainer serviceContainer) override {};

	void OnInstantiateEntity(ecs::EntityID entity_id) override {};

	void OnCleanupEntity(ecs::EntityID entity_id) override {};

	void OnFixedUpdate(double fixed_delta_time, IScene& scene) override {};

	void OnFrameUpdate(double delta_time, double alpha, IScene& scene) override {};
};
