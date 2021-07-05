#pragma once

#include "mesh.h"
#include "material.h"

struct MeshRenderable
{
	// If empty, the shared_mesh is used to define the MeshRenderable's mesh properties. 
	// Set this to a value in order to override this MeshRenderable's mesh properties without affecting other MeshRenderables.
	std::unique_ptr<Mesh> unique_mesh;

	// Mesh that is shared by potentially several MeshRenderables
	std::shared_ptr<Mesh> shared_mesh;

	// If empty, the shared_material is used to define the MeshRenderable's material properties. 
	// Set this to a value in order to override this MeshRenderable's material properties without affecting other MeshRenderables.
	std::unique_ptr<Material> unique_material;

	// Material that is shared by potentially several MeshRenderables
	std::shared_ptr<Material> shared_material;
};