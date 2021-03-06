#pragma once

#include <vector>

#include <core/utils/uid_generator.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

typedef UID MeshID;

class Mesh
{
public:
	// Structure describing data for a single triangle in the mesh.
	typedef struct Triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	} Triangle;

	Mesh(MeshID id);

	~Mesh();

	static Mesh CreateCube(float side_length);

	void SetVertices(std::vector<glm::vec3> verts);

	void SetTriangles(std::vector<Triangle> tris);

	const std::vector<glm::vec3>& GetVertices() 
	{
		return verts_;
	}

	const std::vector<Triangle>& GetTriangles() 
	{
		return tris_;
	}

	const MeshID& GetInstanceID()
	{
		return id_;
	}

private:
	MeshID id_;
	std::vector<glm::vec3> verts_;
	std::vector<Triangle> tris_;
	std::vector<glm::vec3> normals_;
	std::vector<glm::vec2> tex_coords_;
};