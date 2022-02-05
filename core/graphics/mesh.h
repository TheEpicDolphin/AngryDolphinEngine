#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/utils/event_announcer.h>

#include "rendering_pipeline.h"

typedef uint32_t MeshID;

struct VertexAttributeBuffer {
	ShaderDataType data_type;
	std::vector<char> data;
};

struct MeshInfo
{
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
	bool is_static;
};

struct MeshLifecycleEventsListener {

	virtual void MeshAttributeDidChange(Mesh* mesh, std::size_t attribute_index) = 0;

	virtual void MeshDidDestroy(MeshID mesh_id) = 0;
};

class Mesh
{
public:
	// Structure describing data for a single triangle in the mesh.
	struct Triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	};

	Mesh(MeshID id, MeshInfo mesh_info);

	~Mesh();

	const MeshID& GetInstanceID();

	bool IsStatic();

	const std::size_t& VertexCount();

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

	const std::shared_ptr<RenderingPipeline>& GetPipeline();

	const std::vector<VertexAttributeBuffer>& GetVertexAttributeBuffers();

	const VertexAttributeBuffer& GetVertexAttributeBufferAtIndex(std::size_t index);

	void AddLifecycleEventsListener(MeshLifecycleEventsListener* listener);

	void RemoveLifecycleEventsListener(MeshLifecycleEventsListener* listener);

	template<typename T>
	void SetVertexAttributeBufferData(std::string name, std::vector<T> buffer_data)
	{
		std::unordered_map<std::string, std::size_t>::iterator iter = vertex_attribute_buffer_name_map_.find(name);
		if (iter == vertex_attribute_buffer_name_map_.end()) {
			// TODO: print warning that a vertex attribute with this name does not exist for this mesh/pipeline.
			return;
		}
		const std::size_t vab_index = iter->second;

		VertexAttributeBuffer& vabuffer = vertex_attribute_buffers_[vab_index];
		const ShaderDataType data_type = shader::TypeID(buffer_data[0]);
		if (vabuffer.data_type != data_type) {
			// TODO: print warning that the vertex attribute with this name does not have the inputted type.
			return;
		}

		const std::vector<char> buffer_data = shader::BufferData(buffer);
		vabuffer.data = buffer_data;

		lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshAttributeDidChange, this, index);
	}

private:

	MeshID id_;

	bool is_static_;

	std::size_t vertex_count_;

	// Indices to commonly used vertex attributes.
	int position_attribute_index_ = -1;
	int normal_attribute_index_ = -1;
	int tex_coord0_attribute_index_ = -1;
	int bone_weight_attribute_index_ = -1;
	int bone_indices_attribute_index_ = -1;

	std::vector<VertexAttributeBuffer> vertex_attribute_buffers_;
	std::unordered_map<std::string, std::size_t> vertex_attribute_buffer_name_map_;

	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	
	std::vector<Triangle> tris_;

	EventAnnouncer<MeshLifecycleEventsListener> lifecycle_events_announcer_;

	// The input buffer is expected to always have T = glm::(i)vec type, which allows trivial reinterpret_cast from T* to char*.
	template<typename T>
	void SetVertexAttributeBufferWithCachedIndex(int cached_va_index, std::vector<T> buffer)
	{
		char* buffer_data_ptr = reinterpret_cast<char*>(buffer.data());
		const std::vector<char> buffer(buffer_data_ptr, buffer_data_ptr + (buffer.size() * sizeof(T)));
		vertex_attribute_buffers_[cached_va_index].data = buffer;

		lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshAttributeDidChange, this, cached_va_index);
	}

	// The input buffer is expected to always have T = glm::(i)vec type, which allows trivial reinterpret_cast from T* to char*.
	template<typename T>
	std::vector<T> GetVertexAttributeBufferForCachedIndex(int cached_va_index) {
		VertexAttributeBuffer& buffer = vertex_attribute_buffers_[cached_va_index];
		T* buffer_data_ptr = reinterpret_cast<T*>(buffer.data.data());
		const std::vector<T> buffer(buffer_data_ptr, buffer_data_ptr + (buffer.data.size() / sizeof(T)));
		return buffer;
	}
};