#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "mesh.h"

class MeshManager : private MeshDelegate
{
public:
	static std::shared_ptr<Mesh> MeshForResourcePath(const char* resource_path);

	static std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

private:
	void MaterialDidDestruct(Material* material) override;

private:
	std::unordered_map<AssetID, std::shared_ptr<Mesh>> mesh_assets_;
	UIDGenerator mesh_id_generator_;
};