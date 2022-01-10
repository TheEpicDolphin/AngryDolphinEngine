#include "resource_manager.h"

#include <experimental/filesystem>
#include <core/utils/file_utils.h>


namespace fs = std::experimental::filesystem;

std::vector<char>& ResourceManager::LoadAsset(const char* asset_path_name, const char* expected_extension = "*") {
	fs::path asset_path = asset_path_name;
	if (expected_extension != "*") {
		assert(asset_path.extension() == expected_extension);
	}
	std::unordered_map<std::string, AssetID>::iterator iter = asset_id_map_.find(asset_path.filename());
	if (iter != asset_id_map_.end()) {
		// The resource at this path has already been loaded. Return the asset id.
		return loaded_assets_[iter->second];
	}
	else {
		const std::unique_ptr<IAssetLoader> asset_loader = fextension_asset_loader_map_[asset_path.extension()];

		std::vector<char> asset_file_contents = file_helpers::ReadFileWithPath(asset_path);
		asset_id_map_[path] = loaded_assets_.size();
		loaded_assets_.push_back(asset_file_contents);

		const AssetID asset_id = next_asset_id_++;
		asset_id_map_[path] = asset_id;
		return asset_file_contents;
	}
}