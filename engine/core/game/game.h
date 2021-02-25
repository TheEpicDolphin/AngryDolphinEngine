#pragma once

#include <chrono>
#include <core/ecs/ecs.h>
#include <core/graphics/rendering_system.h>
#include <core/simulation/physics_system.h>
#include <core/simulation/physics_interpolation_system.h>

using namespace std::chrono;

class Game {

public:
	Game() {
		rendering_system_ = RenderingSystem(ecs_);
	}

	void Initialize() 
	{
		
	}

	void StartMainLoop() {
		const double fixed_dt = 1/60.0;
		double accumulator = 0.0;
		const time_point<system_clock> start_time = system_clock::now();
		time_point<system_clock> last_tick = start_time;

		while (true) {
			time_point<system_clock> now = system_clock::now();
			double frame_time = fmin(duration<double>(now - last_tick).count(), 0.25);
			last_tick = now;
			accumulator += frame_time;

			while (accumulator >= fixed_dt)
			{
				physics_system_.Update(fixed_dt);
				accumulator -= fixed_dt;
			}

			const double alpha = accumulator / fixed_dt;
			// interpolate physics states to avoid jitter in render
			physics_interp_system_.Update(alpha);

			// render
			rendering_system_.Update();
		}
	}

private:
	ECS ecs_;
	RenderingSystem rendering_system_;
	PhysicsSystem physics_system_;
	PhysicsInterpolationSystem physics_interp_system_;
};