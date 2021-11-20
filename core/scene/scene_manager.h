#pragma once

#include <core/serialize/archive.h>
#include "scene.h"

class SceneManager
{
public:

	void LoadScene(const char* scene_path);

	void SaveScene(Scene& scene);

	std::vector<Scene*>& ActiveScenes();

private:
	// Used for serializing/deserializing the scenes
	Archive archive_;
	std::vector<Scene> scenes_;
};