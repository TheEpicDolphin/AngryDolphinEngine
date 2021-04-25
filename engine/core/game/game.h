#pragma once

#include <chrono>
#include <core/ecs/ecs.h>
#include <core/graphics/rendering_system.h>
#include <core/simulation/physics_system.h>
#include <core/simulation/physics_interpolation_system.h>

using namespace std::chrono;

class Game {

public:
	Game() 
	{
		rendering_system_ = RenderingSystem(&ecs_);
		physics_system_ = PhysicsSystem(&ecs_);
	}

	void StartMainLoop();

private:
	ECS ecs_;
	RenderingSystem rendering_system_;
	PhysicsSystem physics_system_;
	PhysicsInterpolationSystem physics_interp_system_;
};