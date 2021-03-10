#pragma once

#include <iostream>
#include <vector>

#include <core/ecs/component.h>
#include "mesh.h"
#include "material.h"

struct MeshRenderer : public Component<MeshRenderer>
{
	SharedHandle<Mesh> mesh;
	SharedHandle<Material> material;
};