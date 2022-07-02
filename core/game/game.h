#pragma once

#include <chrono>
#include <core/scene/scene_manager.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>

enum class WindowRendererType {
	OpenGL = 0,
	//Vulkan
};

class Game {
public:
	Game(int window_width, int window_height, WindowRendererType window_renderer_type);

	~Game();

	const SceneManager& GetSceneManager()
	{
		return *scene_manager_;
	}

	// Capture the state of the game. Useful for debugging.
	void CaptureSnapshot();

	void StartMainLoop();

private:
	SceneManager* scene_manager_;
	GLFWwindow* window_;
	WindowRendererType window_renderer_type_;
};