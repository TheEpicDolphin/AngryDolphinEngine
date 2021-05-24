#pragma once

#include "mesh.h"
#include "material.h"

struct MeshRenderer
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};