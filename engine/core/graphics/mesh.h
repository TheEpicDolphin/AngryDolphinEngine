#pragma once

#include <iostream>
#include <glm/vec3.hpp>
#include <vector>
#include <core/ecs/typeid_generator.h>
#include <core/object/object.h>

class Mesh : public Object<Mesh>
{
public:
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

	std::vector<glm::vec3> GetVertices() 
	{
		return verts_;
	}

	std::vector<Triangle> GetTriangles() {
		return tris_;
	}

private:
	std::vector<glm::vec3> verts_;
	std::vector<Triangle> tris_;
	std::vector<glm::vec3> normals_;
	std::vector<glm::vec2> tex_coords_;
};