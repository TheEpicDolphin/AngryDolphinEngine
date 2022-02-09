#pragma once

#include <chrono>
#include <core/scene/scene_manager.h>

class Game {

public:
	Game() {}

	const SceneManager& SceneManager()
	{
		return scene_manager_;
	}

	// Capture the state of the game. Useful for debugging.
	void CaptureSnapshot();

	void StartMainLoop();

private:
	SceneManager scene_manager_;
};