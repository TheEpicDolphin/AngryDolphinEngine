
#include "mesh.h"

Mesh::Mesh(MeshID id)
{
	id_ = id;
}

Mesh::~Mesh()
{

}

const std::size_t& Mesh::VertexCount() {
	return vertex_count_;
}

void Mesh::SetMaterial(std::shared_ptr<Material> material)
{
	if (material->GetPipeline()->GetInstanceID() == rendering_pipeline_->GetInstanceID()) {
		material_ = material;
	}
	else {
		// TODO: Print warning
	}
}

const std::shared_ptr<Material>& Mesh::GetMaterial()
{
	return material_;
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

std::vector<glm::vec3> Mesh::GetVertexPositions() {
	VertexAttributeBuffer& buffer = vertex_attribute_buffers_[position_attribute_index_];
	glm::vec3* positions_ptr = reinterpret_cast<glm::vec3*>(buffer.data.data());
	const std::vector<glm::vec3> positions(positions_ptr, positions_ptr + vertex_count_);
	return positions;
}

const std::shared_ptr<RenderingPipeline>& Mesh::GetPipeline()
{
	return rendering_pipeline_;
}

const std::vector<VertexAttributeBuffer>& Mesh::GetVertexAttributeBuffers()
{
	return vertex_attribute_buffers_;
}

void Mesh::SetTriangles(std::vector<Triangle> tris) 
{
	tris_ = tris;
}

const std::vector<Triangle>& Mesh::GetTriangles() {
	return tris_;
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