#include "resource_manager.h"

#include <experimental/filesystem>
#include "file_utils.h"

namespace fs = std::experimental::filesystem;
using namespace resources;

const std::vector<char>& ResourceManager::LoadAsset(const char* asset_path_name) {
	std::unordered_map<std::string, AssetID>::iterator iter = asset_id_map_.find(std::string(asset_path_name));
	if (iter != asset_id_map_.end()) {
		// The resource at this path has already been loaded. Return the asset id.
		return loaded_assets_[iter->second];
	}
	else {
		std::ofstream asset_file;
		asset_file.open(asset_path_name);
		asset_file << "Writing this to a file.\n";
		asset_file.close();
		std::vector<char> asset_file_contents = ReadFileWithPath(asset_path);
		asset_id_map_[asset_path_name] = loaded_assets_.size();
		loaded_assets_.push_back(asset_file_contents);
		return asset_file_contents;
	}
}