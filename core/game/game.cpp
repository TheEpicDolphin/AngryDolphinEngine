#include "game.h"

void Game::LoadScene(const char* scene_path) {

}

void Game::UnloadScene(const char* scene_path) {

}

void Game::CaptureSnapshot() {

}

void Game::StartMainLoop() {
	const double fixed_dt = 1 / 60.0;
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
			for (Scene* scene : scene_manager_.ActiveScenes()) {
				scene->OnFixedUpdate(fixed_dt);
			}
			accumulator -= fixed_dt;
		}

		const double alpha = accumulator / fixed_dt;
		// interpolate physics states to avoid jitter in render
		physics_interp_system_.Update(alpha);

		// render
		rendering_system_.Update();


		for (Scene* scene : scene_manager_.ActiveScenes()) {
			scene->OnUpdate(frame_time, alpha);
		}
	}
}