#pragma once

#include <unordered_map>
#include <core/utils/uid_generator.h>

#include "mesh.h"

class MeshManager : private MeshDelegate
{
public:
	static std::shared_ptr<Mesh> CreateMeshForResourcePath(const char* resource_path);

	static std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

private:
	void MeshDidDestruct(Mesh* mesh) override;

private:
	static std::unique_ptr<UIDGenerator> mesh_id_generator_;
};