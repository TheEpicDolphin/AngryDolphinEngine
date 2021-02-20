#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>

#include <core/ecs/component.h>

typedef std::uint64_t MeshID;

class Mesh : public Component<Mesh>
{
public:
	// Structure describing data for a single triangle in the mesh.
	struct triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	};

	Mesh();

	~Mesh();

	static Mesh CreateCube(float side_length);

	void SetVertices(std::vector<glm::vec3> verts);

	void SetTriangles(std::vector<triangle> tris);

protected:
	std::vector<glm::vec3> verts_;
	std::vector<triangle> tris_;

private:
	MeshID id_;
};