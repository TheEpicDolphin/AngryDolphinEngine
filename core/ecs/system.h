#pragma once

class ServiceContainer;

namespace ecs {
	struct EntityID;
}

// TODO: Extensions of this class will include more lifecycle methods such as WillRenderFrame, DidAnimate, etc.
// TODO: Create wrapper for IScene that prevents reassignment to avoid client reassigning scene reference in methods below.

class ISystem {
public:
	virtual void Initialize(ServiceContainer service_container) = 0;

	virtual void Cleanup(ServiceContainer service_container) = 0;

	virtual void OnFixedUpdate(double fixed_delta_time) = 0;

	virtual void OnFrameUpdate(double delta_time, double alpha) = 0;
};
