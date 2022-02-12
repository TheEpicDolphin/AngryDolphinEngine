#pragma once

#include <core/ecs/system.h>
#include <core/scene/scene.h>

class RenderingSystem : public ISystem
{
public:
	RenderingSystem() = default;

	void OnFrameUpdate(double delta_time, double alpha, IScene& scene) override;
};