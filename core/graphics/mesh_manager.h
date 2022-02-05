#pragma once

#include <unordered_map>

#include "mesh.h"

class MeshManager
{
public:
	static std::shared_ptr<Mesh> CreateMeshForResourcePath(const char* resource_path);

	static std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

	static std::shared_ptr<Mesh> CreateCubeMeshPrimitive(MeshInfo info, float side_length);

private:
	static std::uint32_t next_mesh_id_;
};