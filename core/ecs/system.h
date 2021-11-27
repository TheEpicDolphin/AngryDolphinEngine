#pragma once

class Scene;

class ISystem {
	virtual void DidActivate(const Scene& scene) {};

	virtual void OnFixedUpdate(double fixed_delta_time, const Scene& scene) {};

	virtual void OnFrameUpdate(double delta_time, double alpha, const Scene& scene) {};
};
