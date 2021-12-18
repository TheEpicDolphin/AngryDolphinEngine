
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
	SetVertexAttributeBufferWithCachedIndex<glm::vec3>(VERTEX_ATTRIBUTE_POSITION_NAME, verts, position_attribute_index_);
}

std::vector<glm::vec3> Mesh::GetVertexPositions() {
	VertexAttributeBuffer& buffer = vertex_attribute_buffers_[position_attribute_index_];
	glm::vec3* positions_ptr = reinterpret_cast<glm::vec3*>(buffer.data.data());
	const std::vector<glm::vec3> positions(positions_ptr, positions_ptr + vertex_count_);
	return positions;
}

void Mesh::SetNormals(std::vector<glm::vec3> normals) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec3>(VERTEX_ATTRIBUTE_NORMAL_NAME, normals, normal_attribute_index_);
}

std::vector<glm::vec3> Mesh::GetNormals() {

}

void Mesh::SetTexCoords(std::vector<glm::vec2> tex_coords) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec2>(VERTEX_ATTRIBUTE_TEX_COORDS_NAME, tex_coords, tex_coords_attribute_index_);
}

std::vector<glm::vec2> Mesh::GetTexCoords() {

}

void Mesh::SetBoneWeights(std::vector<glm::vec4> bone_weights) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec4>(VERTEX_ATTRIBUTE_BONE_WEIGHTS_NAME, bone_weights, bone_weights_attribute_index_);
}

std::vector<glm::vec4> Mesh::GetBoneWeights() {

}

void Mesh::SetBoneIndices(std::vector<glm::ivec4> bone_indices) {
	SetVertexAttributeBufferWithCachedIndex<glm::ivec4>(VERTEX_ATTRIBUTE_BONE_INDICES_NAME, bone_indices, bone_indices_attribute_index_);
}

std::vector<glm::ivec4> Mesh::GetBoneIndices() {

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

const std::vector<Mesh::Triangle>& Mesh::GetTriangles() {
	return tris_;
}

std::shared_ptr<Mesh> Mesh::CreateCubePrimitive(float side_length) {
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

	mesh.SetVertexPositions(cube_verts);
	return std::make_shared<Mesh>(mesh);
}