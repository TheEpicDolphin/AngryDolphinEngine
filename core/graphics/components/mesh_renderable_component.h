#pragma once

#include <core/geometry/bounds.h>

#include "../mesh.h"
#include "../material.h"

struct MeshRenderableComponent
{
	bool enabled;
	// If empty, the shared_mesh is used to define the MeshRenderable's mesh properties. 
	// Set this to a value in order to override this MeshRenderable's mesh properties without
	// affecting other MeshRenderables.
	std::unique_ptr<Mesh> mesh;

	// Mesh that is shared by potentially several MeshRenderables
	std::shared_ptr<Mesh> shared_mesh;


	std::unique_ptr<Material> material;

	// Material that is shared by potentially several MeshRenderables
	std::shared_ptr<Material> shared_material;

	const geometry::Bounds& WorldMeshBounds() {
		return world_mesh_bounds_;
	}

private:

	// Bounds of the mesh in world space
	geometry::Bounds world_mesh_bounds_;

	friend class MeshTransformationSystem;
};