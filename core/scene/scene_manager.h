#pragma once

#include <core/serialize/archive.h>
#include <core/definitions/graphics/renderer.h>
#include <core/definitions/scene/scene_service.h>
#include <core/services/service_container.h>

#include "scene.h"

class SceneManager : public ISceneService
{
public:
	SceneManager(IRenderer* renderer);

	~SceneManager();

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

	ServiceContainer service_container_;
};