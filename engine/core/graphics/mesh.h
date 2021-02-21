#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>
#include <core/ecs/uid_generator.h>

typedef UID MeshID;

class Mesh
{
public:
	MeshID id_;

	// Structure describing data for a single triangle in the mesh.
	struct Triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	};

	Mesh();

	~Mesh();

	static Mesh CreateCube(float side_length);

	void SetVertices(std::vector<glm::vec3> verts);

	void SetTriangles(std::vector<Triangle> tris);

private:
	std::vector<glm::vec3> verts_;
	std::vector<Triangle> tris_;
	std::vector<glm::vec3> normals_;
	std::vector<glm::vec2> tex_coords_;
};