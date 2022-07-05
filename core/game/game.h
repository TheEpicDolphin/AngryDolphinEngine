#pragma once

#include <chrono>
#include <core/scene/scene.h>
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

	// Capture the state of the game. Useful for debugging.
	void CaptureSnapshot();

	void PlayMainScene(IScene* main_scene);

private:
	SceneManager* scene_manager_;
	GLFWwindow* window_;
	WindowRendererType window_renderer_type_;
};