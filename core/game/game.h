#pragma once

#include <chrono>
#include <core/ecs/ecs.h>
#include <core/graphics/rendering_system.h>
#include <core/simulation/physics_system.h>
#include <core/simulation/physics_interpolation_system.h>

using namespace std::chrono;

class Game {

public:
	Game(const char* initial_scene_path);

	void LoadScene(const char* scene_path);

	void UnloadScene(const char* scene_path);

	// Capture the state of the game. Useful for debugging.
	void CaptureSnapshot();

	void StartMainLoop();

private:
	SceneManager scene_manager_;
};