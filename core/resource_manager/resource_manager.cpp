
#include "resource_manager.h"

#include <fstream>
#include <iostream>
#include <config/generated/config.h>

using namespace resource_manager;

std::vector<char> ReadAssetFile(const char* path) {
	std::ifstream asset_file_istream;
	asset_file_istream.open(path, std::ios::in);

	if (!asset_file_istream.good()) {
		// Failed to open the file. Return empty vector.
		return std::vector<char>();
	}

	std::vector<char> asset_file_contents((std::istreambuf_iterator<char>(asset_file_istream)), std::istreambuf_iterator<char>());
	asset_file_contents.push_back('\0');
	asset_file_istream.close();
	return asset_file_contents;
}

std::vector<char> ResourceManager::LoadAsset(const char* asset_path_name) {
	static std::unordered_map<std::string, AssetID> asset_id_map;
	static std::vector<std::vector<char>> loaded_assets;

	std::string asset_path_name_string = asset_path_name;
	std::unordered_map<std::string, AssetID>::iterator iter = asset_id_map.find(asset_path_name_string);
	if (iter != asset_id_map.end()) {
		// The resource at this path has already been loaded. Return the asset id.
		return loaded_assets[iter->second];
	}
	else {
		const int num_project_resources_directories = sizeof(config::project_resources_directories) / sizeof(config::project_resources_directories[0]);
		for (std::size_t i = 0; i < num_project_resources_directories; i++) {
			std::string resources_asset_path(config::project_resources_directories[i]);
			resources_asset_path.append("/");
			resources_asset_path.append(asset_path_name);
			std::vector<char> asset_file_contents = ReadAssetFile(resources_asset_path.c_str());
			if (!asset_file_contents.empty()) {
				// We found the asset file.
				asset_id_map[asset_path_name_string] = loaded_assets.size();
				loaded_assets.push_back(asset_file_contents);
				return asset_file_contents;
			}
		}

		// Asset not found. Return empty vector.
		return std::vector<char>();
	}
}