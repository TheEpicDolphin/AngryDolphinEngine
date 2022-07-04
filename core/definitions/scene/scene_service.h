#pragma once

class ISceneService {
public:
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
};