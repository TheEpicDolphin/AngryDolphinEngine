#include "game.h"

void Game::CaptureSnapshot() {

}

void Game::StartMainLoop() {
	const double fixed_dt = 1 / 60.0;
	double accumulator = 0.0;
	const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> last_tick = start_time;

	while (true) {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		double frame_time = fmin(std::chrono::duration<double>(now - last_tick).count(), 0.25);
		last_tick = now;
		accumulator += frame_time;

		while (accumulator >= fixed_dt)
		{
			for (IScene* scene : scene_manager_.LoadedScenes()) {
				scene->OnFixedUpdate(fixed_dt);
			}
			accumulator -= fixed_dt;
		}

		const double alpha = accumulator / fixed_dt;
		for (IScene* scene : scene_manager_.LoadedScenes()) {
			scene->OnFrameUpdate(frame_time, alpha);
		}
	}
}