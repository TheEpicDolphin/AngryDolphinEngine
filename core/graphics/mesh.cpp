
#include "mesh.h"

Mesh::Mesh(MeshInfo info)
{
	rendering_pipeline_ = info.rendering_pipeline;
	is_static_ = info.is_static;

	vertex_attribute_buffers_ = {};
	for (VertexAttributeInfo vertex_attribute_info : rendering_pipeline_->VertexAttributes()) {
		const std::size_t index = vertex_attribute_buffers_.size();
		vertex_attribute_buffers_.push_back({
			vertex_attribute_info.data_type,
			std::vector<char>()
			});
		vertex_attribute_buffer_name_map_[vertex_attribute_info.name] = index;
		switch (vertex_attribute_info.category)
		{
		case VertexAttributeUsageCategory::Position:
			position_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategory::Normal:
			normal_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategory::TexCoord0:
			tex_coord0_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategory::BoneWeight:
			bone_weight_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategory::BoneIndices:
			bone_indices_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategory::Custom:
			// Do nothing
			break;
		}
	}
}

std::shared_ptr<Mesh> Mesh::CreateMesh(MeshInfo info) {
	return std::make_shared<Mesh>(info);
}

std::shared_ptr<Mesh> Mesh::CreateCubeMeshPrimitive(MeshInfo info, glm::vec3 origin, float side_length) {
	std::shared_ptr<Mesh> mesh = CreateMesh(info);
	std::vector<glm::vec3> cube_verts =
	{
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(0, 0, 1)
	};

	const std::vector<unsigned int> indices =
	{
		0, 2, 1, //face front
		0, 3, 2,
		2, 3, 4, //face top
		2, 4, 5,
		1, 2, 5, //face right
		1, 5, 6,
		0, 7, 4, //face left
		0, 4, 3,
		5, 4, 7, //face back
		5, 7, 6,
		0, 6, 7, //face bottom
		0, 1, 6
	};

	for (std::vector<glm::vec3>::iterator it = cube_verts.begin(); it != cube_verts.end(); ++it) {
		*it = (*it - glm::vec3(0.5f, 0.5f, 0.5f) + origin) * side_length;
	}

	mesh->SetVertexPositions(cube_verts);
	mesh->SetTriangleIndices(indices);
	return mesh;
}

Mesh::~Mesh()
{
	lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshDidDestroy, this);
}

bool Mesh::IsStatic() {
	return is_static_;
}

const std::size_t& Mesh::VertexCount() {
	return vertex_attribute_buffers_[position_attribute_index_].data.size() / sizeof(glm::vec3);
}

void Mesh::SetVertexPositions(std::vector<glm::vec3> verts) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec3>(position_attribute_index_, verts);
}

std::vector<glm::vec3> Mesh::GetVertexPositions() {
	return GetVertexAttributeBufferForCachedIndex<glm::vec3>(position_attribute_index_);
}

void Mesh::SetNormals(std::vector<glm::vec3> normals) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec3>(normal_attribute_index_, normals);
}

std::vector<glm::vec3> Mesh::GetNormals() {
	return GetVertexAttributeBufferForCachedIndex<glm::vec3>(normal_attribute_index_);
}

void Mesh::SetTexCoords(std::vector<glm::vec2> tex_coords) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec2>(tex_coord0_attribute_index_, tex_coords);
}

std::vector<glm::vec2> Mesh::GetTexCoords() {
	return GetVertexAttributeBufferForCachedIndex<glm::vec2>(tex_coord0_attribute_index_);
}

void Mesh::SetBoneWeights(std::vector<glm::vec4> bone_weights) {
	SetVertexAttributeBufferWithCachedIndex<glm::vec4>(bone_weight_attribute_index_, bone_weights);
}

std::vector<glm::vec4> Mesh::GetBoneWeights() {
	return GetVertexAttributeBufferForCachedIndex<glm::vec4>(bone_weight_attribute_index_);
}

void Mesh::SetBoneIndices(std::vector<glm::ivec4> bone_indices) {
	SetVertexAttributeBufferWithCachedIndex<glm::ivec4>(bone_indices_attribute_index_, bone_indices);
}

std::vector<glm::ivec4> Mesh::GetBoneIndices() {
	return GetVertexAttributeBufferForCachedIndex<glm::ivec4>(bone_indices_attribute_index_);
}

const std::shared_ptr<RenderingPipeline>& Mesh::GetPipeline()
{
	return rendering_pipeline_;
}

const std::vector<VertexAttributeBuffer>& Mesh::GetVertexAttributeBuffers()
{
	return vertex_attribute_buffers_;
}

const VertexAttributeBuffer& Mesh::GetVertexAttributeBufferAtIndex(std::size_t index)
{
	return vertex_attribute_buffers_[index];
}

void Mesh::SetTriangleIndices(std::vector<unsigned int> tri_indices) {
	// This does not affect the world mesh bounds! Yay!
	triangle_indices_ = tri_indices;
}

const std::vector<unsigned int>& Mesh::GetTriangleIndices() {
	return triangle_indices_;
}

void Mesh::AddLifecycleEventsListener(MeshLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.AddListener(listener);
}

void Mesh::RemoveLifecycleEventsListener(MeshLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.RemoveListener(listener);
}