
#include "mesh.h"

Mesh::Mesh(MeshID id, MeshInfo info)
{
	id_ = id;
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
		case VertexAttributeUsageCategoryPosition:
			position_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategoryNormal:
			normal_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategoryTexCoord0:
			tex_coord0_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategoryBoneWeight:
			bone_weight_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategoryBoneIndices:
			bone_indices_attribute_index_ = (int)index;
			break;
		case VertexAttributeUsageCategoryCustom:
			// Do nothing
			break;
		}
	}
}

Mesh::~Mesh()
{
	lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshDidDestroy, this->GetInstanceID());
}

const MeshID& Mesh::GetInstanceID() {
	return id_;
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

void Mesh::SetTriangleIndices(std::vector<std::size_t> tri_indices) {
	// This does not affect the world mesh bounds! Yay!
	triangle_indices_ = tri_indices;
}

const std::vector<std::size_t>& Mesh::GetTriangleIndices() {
	return triangle_indices_;
}

void Mesh::AddLifecycleEventsListener(MeshLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.AddListener(listener);
}

void Mesh::RemoveLifecycleEventsListener(MeshLifecycleEventsListener* listener) {
	lifecycle_events_announcer_.RemoveListener(listener);
}