#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>
#include <core/resources/resource_manager.h>

#include "material.h"
#include "rendering_pipeline_manager.h"

class MaterialManager : private MaterialDelegate
{
public:
	static std::shared_ptr<Material> MaterialForResourcePath(const char* resource_path);

	static std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

private:
	void MaterialDidDestruct(Material *material) override;

private:
	UIDGenerator material_id_generator_;
};