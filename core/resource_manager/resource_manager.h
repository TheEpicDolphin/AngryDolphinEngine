#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

namespace resource_manager {
	typedef std::size_t AssetID;

	// This is a static class. Handles loading files from project's resources folder.
	class ResourceManager
	{
	public:
		/// <summary>
		/// Loads asset in resources folder.
		/// </summary>
		/// <param name="asset_path_name">Path of asset relative to the resources folder.</param>
		/// <returns>Asset file contents</returns>
		static std::vector<char> LoadAsset(const char* asset_path_name);
	};
}
