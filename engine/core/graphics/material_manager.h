#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "material.h"

class MaterialManager : public MaterialDelegate
{
public:
	// Searches the Resources folder for any .materialspec files and creates Material objects from them.
	void LoadMaterialSpecs();

	// Calculates hash for the specified material spec file path. This allows for easier material fetching.
	static int MaterialSpecHashForFilePath(std::string);

	std::shared_ptr<Material> MaterialForMaterialSpecHash(int hash);

	void MaterialDidDestruct(Material *material) override;

private:
	std::unordered_map<int, std::shared_ptr<Material>> spec_generated_materials_;
	UIDGenerator material_id_generator_;
	ShaderManager *shader_manager_;
};