#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <core/utils/event_announcer.h>

#include "shader/shader_vars/shader_data_type.h"
#include "shader/shader_vars/shader_var_helpers.h"
#include "rendering_pipeline.h"

struct VertexAttributeBuffer {
	shader::ShaderDataType data_type;
	std::vector<char> data;
};

struct MeshInfo
{
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
	bool is_static;
};

class Mesh;

struct MeshLifecycleEventsListener {
	virtual void MeshVertexAttributeDidChange(Mesh* mesh, std::size_t attribute_index) = 0;
	virtual void MeshDidDestroy(Mesh* mesh) = 0;
};

class Mesh
{
public:
	// Meshes can only be created using factory methods.

	static std::shared_ptr<Mesh> CreateMesh(MeshInfo info);

	static std::shared_ptr<Mesh> CreateCubeMeshPrimitive(MeshInfo info, glm::vec3 origin, float side_length);

	~Mesh();

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

	void SetTriangleIndices(std::vector<std::size_t> tri_indices);

	const std::vector<std::size_t>& GetTriangleIndices();

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
		const shader::ShaderDataType data_type = shader::TypeID(buffer_data[0]);
		if (vabuffer.data_type != data_type) {
			// TODO: print warning that the vertex attribute with this name does not have the inputted type.
			return;
		}

		vabuffer.data = shader::DataFromBuffer(buffer);

		lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshVertexAttributeDidChange, this, index);
	}

private:
	bool is_static_;

	// Indices to commonly used vertex attributes.
	int position_attribute_index_ = -1;
	int normal_attribute_index_ = -1;
	int tex_coord0_attribute_index_ = -1;
	int bone_weight_attribute_index_ = -1;
	int bone_indices_attribute_index_ = -1;

	// These elements are indices into the vertex positions. 
	// Elements n, n+1, and n+2, where n % 3 == 0, form a triangle going clockwise.
	std::vector<std::size_t> triangle_indices_;

	std::vector<VertexAttributeBuffer> vertex_attribute_buffers_;
	std::unordered_map<std::string, std::size_t> vertex_attribute_buffer_name_map_;

	std::shared_ptr<RenderingPipeline> rendering_pipeline_;

	EventAnnouncer<MeshLifecycleEventsListener> lifecycle_events_announcer_;

	Mesh(MeshInfo mesh_info);

	// The input buffer is expected to always have T = glm::(i)vec type, which allows trivial reinterpret_cast from T* to char*.
	template<typename T>
	void SetVertexAttributeBufferWithCachedIndex(int cached_va_index, std::vector<T> buffer)
	{
		vertex_attribute_buffers_[cached_va_index].data = shader::DataFromBuffer(buffer);
		lifecycle_events_announcer_.Announce(&MeshLifecycleEventsListener::MeshVertexAttributeDidChange, this, cached_va_index);
	}

	// The input buffer is expected to always have T = glm::(i)vec type, which allows trivial reinterpret_cast from T* to char*.
	template<typename T>
	std::vector<T> GetVertexAttributeBufferForCachedIndex(int cached_va_index) {
		VertexAttributeBuffer& va_buffer = vertex_attribute_buffers_[cached_va_index];
		return shader::BufferFromData<T>(va_buffer.data);
	}
};