#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

typedef std::size_t AssetID;

// This is a static class. Handles loading files from project's resources folder.
class ResourceManager
{
public:
	/// <summary>
	/// Loads asset in resources folder.
	/// </summary>
	/// <param name="asset_path_name">Path of asset relative to the resources folder.</param>
	/// <param name="expected_extension">Expected extension of asset filename. If it doesn't match, an error
	/// is thrown.</param>
	/// <returns>Asset file contents</returns>
	static std::vector<char> LoadAsset(const char* asset_path_name, const char* expected_extension = "*");

private:
	static std::unordered_map<std::string, AssetID> asset_id_map_;
	std::vector<std::vector<char>> loaded_assets_;
};