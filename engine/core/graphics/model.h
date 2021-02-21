#pragma once

#include <iostream>
#include <vector>

#include <core/ecs/component.h>
#include "mesh.h"
#include "material.h"

class Model : public Component<Model>
{
public:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};