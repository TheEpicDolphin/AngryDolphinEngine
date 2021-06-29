#pragma once

#include "mesh.h"
#include "material.h"

struct MeshRenderable
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};