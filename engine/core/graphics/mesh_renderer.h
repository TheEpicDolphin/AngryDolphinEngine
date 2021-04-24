#pragma once

#include <core/ecs/component.h>
#include "mesh.h"
#include "material.h"

struct MeshRenderer : public Component<MeshRenderer>
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};