#pragma once

#include <unordered_map>

#include "mesh.h"

class MeshManager
{
public:
	// TODO: implement this
	//static std::shared_ptr<Mesh> CreateMeshForResourcePath(const char* resource_path);

	static std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

	static std::shared_ptr<Mesh> CreateCubeMeshPrimitive(MeshInfo info, glm::vec3 origin = glm::vec3(0, 0, 0), float side_length = 1.0f);

private:
	static std::uint32_t next_mesh_id_;
};