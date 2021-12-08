#pragma once

#include "mesh.h"
#include "material.h"

struct SkeletalMeshRenderable
{
	// If empty, the shared_mesh is used to define the MeshRenderable's mesh properties. 
	// Set this to a value in order to override this SkeletalMeshRenderable's mesh properties 
	// without affecting other SkeletalMeshRenderable.
	std::unique_ptr<Mesh> unique_mesh;

	// Mesh that is shared by potentially several MeshRenderables
	std::shared_ptr<Mesh> shared_mesh;

	bool enabled;

	std::vector<EntityID> bones;
};