#pragma once

#include <core/serialize/archive.h>
#include <core/definitions/graphics/renderer.h>

#include "scene.h"

class SceneManager
{
public:
	SceneManager(IRenderer* renderer);

	~SceneManager();

	/*
	 * scene_name: Name of the empty scene to create. The created scene is not loaded.
	*/
	IScene* CreateSimpleScene(const char* scene_name);

	/*
	 * scene_path: Path of the Scene to load.
	*/ 
	void LoadScene(const char* scene_path);

	void LoadScene(IScene* scene);

	/*
	 * Destroys all entities in the given scene and removes it from the SceneManager.
	*/
	void UnloadScene(IScene* scene);


	void SaveScene(IScene* scene);


	IScene* GetLoadedSceneWithName(const char* name);

	const std::vector<IScene*>& LoadedScenes();

private:
	// Used for serializing/deserializing the scenes
	//Archive archive_;

	std::vector<IScene*> loaded_scenes_;

	IRenderer* renderer_;
};