
#include "scene_manager.h"

#include <algorithm>

#include "simple_scene.h"

SceneManager::SceneManager(IRenderer* renderer) {
	renderer_ = renderer;
}

SceneManager::~SceneManager() {
	delete renderer_;
}

IScene* SceneManager::CreateSimpleScene(const char* scene_name) {
	// TODO: Use scene name somehow.
	return new SimpleScene(scene_name, renderer_);
}

void SceneManager::LoadScene(const char* scene_path) {
	// TODO: Load scene from resources folder using deserialization.
}

void SceneManager::LoadScene(IScene* scene) {
	loaded_scenes_.push_back(scene);
	scene->OnLoad();
}

void SceneManager::UnloadScene(IScene* scene) {
	loaded_scenes_.erase(std::remove(loaded_scenes_.begin(), loaded_scenes_.end(), scene), loaded_scenes_.end());
	scene->OnUnload();
}

void SceneManager::SaveScene(IScene* scene) {
	// TODO: Implement serialization.
}

IScene* SceneManager::GetLoadedSceneWithName(const char* name) {
	for (IScene* scene : loaded_scenes_) {
		if (std::string(scene->Name()) == std::string(name)) {
			return scene;
		}
	}
	return nullptr;
}

const std::vector<IScene*>& SceneManager::LoadedScenes() {
	return loaded_scenes_;
}