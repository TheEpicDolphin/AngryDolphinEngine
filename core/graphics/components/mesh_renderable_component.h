#pragma once

#include <core/geometry/bounds.h>

#include "../mesh.h"
#include "../material.h"

struct MeshRenderableComponent
{
	bool disabled;

	// Mesh that is potentially shared by several MeshRenderables
	std::shared_ptr<Mesh> mesh;

	// Material that is potentially shared by several MeshRenderables
	std::shared_ptr<Material> material;

	const geometry::Bounds& WorldMeshBounds() {
		return world_mesh_bounds_;
	}

private:

	// Bounds of the mesh in world space
	geometry::Bounds world_mesh_bounds_;

	friend class MeshTransformationSystem;
};