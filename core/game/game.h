#pragma once

#include <chrono>
#include <core/scene/scene_manager.h>

class Game {
public:
	Game(RendererType renderer_type) : scene_manager_(renderer_type) {}

	SceneManager& GetSceneManager()
	{
		return scene_manager_;
	}

	// Capture the state of the game. Useful for debugging.
	void CaptureSnapshot();

	void StartMainLoop();

private:
	SceneManager scene_manager_;
};