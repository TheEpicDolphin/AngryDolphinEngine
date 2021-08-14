
#include "mesh.h"

Mesh::Mesh()
{

}

Mesh::~Mesh()
{

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

void Mesh::SetVertexPositions(std::vector<glm::vec3> verts) {
	if (!position_attribute_index_) {
		position_attribute_index_ = vertex_attribute_buffers_.size();
		SetVertexAttributeBuffer<glm::vec3>(VERTEX_ATTRIBUTE_POSITION_NAME, verts);
	}
	else {
		vertex_attribute_buffers_[position_attribute_index_].data = shader::BufferData(verts);
		vertex_attribute_buffers_[position_attribute_index_].is_dirty = true;
	}
}

std::size_t Mesh::VertexCount() {
	return;
}

void Mesh::SetTriangles(std::vector<Triangle> tris) 
{
	tris_ = tris;
}