
#include "mesh.h"

Mesh::Mesh()
{
	id_ = UIDGenerator<Mesh>().CheckoutNewId();
}

Mesh::~Mesh()
{
	UIDGenerator<Mesh>().ReturnId(id_);
	id_ = 0;
}

Mesh Mesh::CreateCube(float side_length) {
	Mesh mesh;
	std::vector<glm::vec3> cube_verts = 
	{
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
	};

	for (std::vector<glm::vec3>::iterator it = cube_verts.begin(); it != cube_verts.end(); ++it) {
		*it = *it * side_length;
	}

	mesh.SetVertices(cube_verts);
	return mesh;
}

void Mesh::SetVertices(std::vector<glm::vec3> verts) 
{
	verts_ = verts;
}

void Mesh::SetTriangles(std::vector<Triangle> tris) 
{
	tris_ = tris;
}