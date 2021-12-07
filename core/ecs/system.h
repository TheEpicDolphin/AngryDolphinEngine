#pragma once

class IScene;

// TODO: Extensions of this class will include more lifecycle methods such as WillRenderFrame, DidAnimate, etc.

class ISystem {
	virtual void DidActivate(const IScene& scene) {};

	virtual void OnFixedUpdate(double fixed_delta_time, const IScene& scene) {};

	virtual void OnFrameUpdate(double delta_time, double alpha, const IScene& scene) {};
};
