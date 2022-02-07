#pragma once

#include "mesh.h"

struct SkeletalMeshRenderableComponent
{
	// If empty, the shared_mesh is used to define the MeshRenderable's mesh properties. 
	// Set this to a value in order to override this SkeletalMeshRenderable's mesh properties 
	// without affecting other SkeletalMeshRenderable.
	std::unique_ptr<Mesh> unique_mesh;

	// Mesh that is shared by potentially several SkeletalMeshRenderables
	std::shared_ptr<Mesh> shared_mesh;

	bool enabled;

	//std::vector<ecs::EntityID> bones;
};