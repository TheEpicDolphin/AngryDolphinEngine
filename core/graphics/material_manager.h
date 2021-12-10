#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "material.h"
#include "rendering_pipeline_manager.h"

class MaterialManager : public MaterialDelegate
{
public:
	// Searches the Resources folder for any .materialspec files and creates Material objects from them.
	void LoadMaterialSpecs();

	// Calculates hash for the specified material spec file path. This allows for easier material fetching.
	static int MaterialSpecHashForFilePath(std::string);

	std::shared_ptr<Material> MaterialForMaterialSpecHash(int hash);

	void MaterialDidDestruct(Material *material) override;

	std::unique_ptr<Material> CreateUniqueMaterial(MaterialInfo info);

	std::shared_ptr<Material> CreateSharedMaterial(MaterialInfo info);

private:
	std::unordered_map<int, std::shared_ptr<Material>> spec_generated_materials_;
	UIDGenerator material_id_generator_;
	RenderingPipelineManager* pipeline_manager_;
};