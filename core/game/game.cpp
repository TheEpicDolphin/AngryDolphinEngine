#include "game.h"

#include "core/graphics/opengl_renderer.h"

void PrepareWindowForFrameRender(WindowRendererType window_renderer_type, GLFWwindow* window) {
	switch (window_renderer_type)
	{
	case WindowRendererType::OpenGL:
		glfwSwapBuffers(window);
		glfwPollEvents();
		break;
	}
}

Game::Game(int window_width, int window_height, WindowRendererType window_renderer_type) {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window_ = glfwCreateWindow(window_width, window_height, "Tutorial 01", NULL, NULL);
	if (window_ == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window_);

	window_renderer_type_ = window_renderer_type;
	switch (window_renderer_type_)
	{
	case WindowRendererType::OpenGL:
		// Initialize GLEW
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			getchar();
			glfwTerminate();
			return;
		}

		// Dark blue background
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

		scene_manager_ = new SceneManager(new OpenGLRenderer());
		break;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window_, GLFW_STICKY_KEYS, GL_TRUE);
}

Game::~Game() {
	glfwDestroyWindow(window_);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void Game::CaptureSnapshot() {

}

void Game::PlayMainScene(IScene* main_scene) {
	scene_manager_->LoadScene(main_scene);
	std::vector<IScene*> loaded_scenes_from_last_frame;

	std::cout << "starting game loop..." << std::endl;
	const double fixed_dt = 1 / 60.0;
	double accumulator = 0.0;
	std::chrono::time_point<std::chrono::system_clock> last_tick = std::chrono::system_clock::now();
	while (glfwGetKey(window_, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window_) == 0) {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		double frame_time = fmin(std::chrono::duration<double>(now - last_tick).count(), 0.25);
		last_tick = now;
		accumulator += frame_time;

		while (accumulator >= fixed_dt)
		{
			// Perform fixed update on all loaded scenes from the last frame update.
			for (IScene* scene : loaded_scenes_from_last_frame) {
				scene->OnFixedUpdate(fixed_dt);
			}
			accumulator -= fixed_dt;
		}

		const double alpha = accumulator / fixed_dt;
		PrepareWindowForFrameRender(window_renderer_type_, window_);
		
		// Perform frame render update on all currently loaded scenes.
		loaded_scenes_from_last_frame.clear();
		const std::vector<IScene*>& current_loaded_scenes = scene_manager_->LoadedScenes();
		loaded_scenes_from_last_frame.insert(loaded_scenes_from_last_frame.end(), current_loaded_scenes.begin(), current_loaded_scenes.end());
		for (IScene* scene : loaded_scenes_from_last_frame) {
			scene->OnFrameUpdate(frame_time, alpha);
		}
	}

	std::cout << "exited game loop" << std::endl;
}