
#include "scene_manager.h"

#include <core/graphics/opengl_renderer.h>

#include "simple_scene.h"

SceneManager::SceneManager(RendererType renderer_type) {
	switch (renderer_type)
	{
	case RendererTypeOpenGL:
		renderer_ = new OpenGLRenderer();
		break;
	}
}

SceneManager::~SceneManager() {
	delete renderer_;
}

IScene& SceneManager::CreateSimpleScene(const char* scene_name) {
	return SimpleScene(renderer_);
}

void SceneManager::LoadScene(const char* scene_path) {

}

void SceneManager::LoadScene(IScene& scene) {

}

void SceneManager::UnloadScene(IScene& scene) {

}

void SceneManager::SaveScene(IScene& scene) {

}

IScene& SceneManager::GetLoadedSceneAtIndex(std::size_t index) {
	return *loaded_scenes_[index];
}

const std::vector<IScene*>& SceneManager::LoadedScenes() {
	return loaded_scenes_;
}