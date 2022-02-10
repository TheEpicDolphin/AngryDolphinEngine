#pragma once

class IScene;

// TODO: Extensions of this class will include more lifecycle methods such as WillRenderFrame, DidAnimate, etc.
// TODO: Create wrapper for IScene that prevents reassignment to avoid client reassigning scene reference in methods below.

class ISystem {
	virtual void DidActivate(IScene& scene) {};

	virtual void OnFixedUpdate(double fixed_delta_time, IScene& scene) {};

	virtual void OnFrameUpdate(double delta_time, double alpha, IScene& scene) {};
};
