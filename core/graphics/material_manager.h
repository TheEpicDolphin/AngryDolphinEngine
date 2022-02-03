#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>
#include <core/resources/resource_manager.h>

#include "material.h"

class MaterialManager
{
public:
	static std::shared_ptr<Material> CreateMaterialForResourcePath(const char* resource_path);

	static std::shared_ptr<Material> CreateMaterial(MaterialInfo info);

private:
	static std::uint32_t next_material_id_;
};