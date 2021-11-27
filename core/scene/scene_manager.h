#pragma once

#include <core/serialize/archive.h>
#include "scene.h"

class SceneManager
{
public:
	/*
	 * scene_name: Name of the empty scene to create. The created scene is not loaded.
	*/
	Scene& CreateScene(const char* scene_name);

	/*
	 * scene_path: Path of the Scene to load.
	*/ 
	void LoadScene(const char* scene_path);

	void LoadScene(Scene& scene);

	/*
	 * Destroys all entities in the given scene and removes it from the SceneManager.
	*/
	void UnloadScene(Scene& scene);


	void SaveScene(Scene& scene);


	Scene& GetLoadedSceneAtIndex(std::size_t index);

private:
	// Used for serializing/deserializing the scenes
	Archive archive_;

	std::vector<Scene> loaded_scenes_;
};