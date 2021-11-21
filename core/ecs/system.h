#pragma once

class Scene;

class ISystem {
	virtual void DidActivate(const Scene& scene) {};
};

class IFixedUpdateSystem : public ISystem {
	virtual void OnFixedUpdate(double fixed_delta_time, const Scene& scene) = 0;
};

class IFrameUpdateSystem : public ISystem {
	virtual void OnFrameUpdate(double delta_time, double alpha, const Scene& scene) = 0;
};