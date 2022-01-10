#pragma once

#include <vector>

#include <core/utils/uid_generator.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "rendering_pipeline.h"
#include "material.h"

typedef UID MeshID;

struct MeshDelegate
{
	virtual void MeshDidDestruct(Mesh* mesh) = 0;
};

struct VertexAttributeBuffer {
	std::size_t attribute_index;
	ShaderDataType type;
	std::vector<char> data;
	bool is_dirty;
};

struct MeshInfo
{
	std::unordered_map<std::string, VertexAttributeBuffer> vertex_attribute_settings;
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
	bool is_static;
};

class Mesh
{
public:
	// Structure describing data for a single triangle in the mesh.
	struct Triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	};

	Mesh(MeshID id, MeshDelegate *delegate);

	~Mesh();

	const MeshID& GetInstanceID()
	{
		return id_;
	}

	const std::size_t& VertexCount();

	void SetMaterial(std::shared_ptr<Material> material);

	const std::shared_ptr<Material>& GetMaterial();

	void SetVertexPositions(std::vector<glm::vec3> verts);

	std::vector<glm::vec3> GetVertexPositions();

	void SetNormals(std::vector<glm::vec3> verts);

	std::vector<glm::vec3> GetNormals();

	void SetTexCoords(std::vector<glm::vec2> tex_coords);

	std::vector<glm::vec2> GetTexCoords();

	void SetBoneWeights(std::vector<glm::vec4> bone_weights);

	std::vector<glm::vec4> GetBoneWeights();

	void SetBoneIndices(std::vector<glm::ivec4> bone_indices);

	std::vector<glm::ivec4> GetBoneIndices();

	void SetTriangles(std::vector<Triangle> tris);

	const std::vector<Triangle>& GetTriangles();

	template<typename T>
	void SetVertexAttributeBuffer(std::string name, std::vector<T> buffer)
	{
		const ShaderDataType type = shader::TypeID(buffer[0]);
		const std::vector<char> buffer_data = shader::BufferData(buffer);
		// Check if rendering pipeline actually has a uniform with this name and type.
		const std::size_t index = rendering_pipeline_->IndexOfVertexAttributeWithNameAndType(name, type);
		if (index != shader::index_not_found) {
			vertex_attribute_buffer_index_map_[name] = vertex_attribute_buffers_.size();
			vertex_attribute_buffers_.push_back({ index, type, num_components, data_type_size, buffer_data, true });
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	const std::shared_ptr<RenderingPipeline>& GetPipeline();

	const std::vector<VertexAttributeBuffer>& GetVertexAttributeBuffers();

	static std::shared_ptr<Mesh> CreateCubePrimitive(float side_length);

private:

	MeshID id_;

	std::shared_ptr<Material> material_;

	std::size_t vertex_count_;

	// Indices to commonly used vertex attributes.
	int position_attribute_index_ = -1;
	int normal_attribute_index_ = -1;
	int tex_coords_attribute_index_ = -1;
	int bone_weights_attribute_index_ = -1;
	int bone_indices_attribute_index_ = -1;

	std::vector<VertexAttributeBuffer> vertex_attribute_buffers_;
	std::unordered_map<std::string, std::size_t> vertex_attribute_buffer_index_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	
	std::vector<Triangle> tris_;

	std::shared_ptr<MeshDelegate> delegate_;

	template<typename T>
	void SetVertexAttributeBufferWithCachedIndex(std::string name, std::vector<T> buffer, int& cached_va_index)
	{
		if (cached_va_index < 0) {
			cached_va_index = vertex_attribute_buffers_.size();
			SetVertexAttributeBuffer<T>(name, buffer);
		}
		else {
			vertex_attribute_buffers_[cached_va_index].data = shader::BufferData(buffer);
			vertex_attribute_buffers_[cached_va_index].is_dirty = true;
		}
	}

	template<typename T>
	std::vector<T> GetVertexAttributeBufferWithCachedIndex(int& cached_va_index) {
		VertexAttributeBuffer& buffer = vertex_attribute_buffers_[position_attribute_index_];
		glm::vec3* positions_ptr = reinterpret_cast<glm::vec3*>(buffer.data.data());
		const std::vector<glm::vec3> positions(positions_ptr, positions_ptr + vertex_count_);
		return positions;
	}
};