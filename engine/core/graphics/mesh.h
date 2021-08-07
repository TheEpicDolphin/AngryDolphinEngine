#pragma once

#include <vector>

#include <core/utils/uid_generator.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include "rendering_pipeline.h"

typedef UID MeshID;

struct MeshDelegate
{
	virtual void MaterialDidDestruct(Mesh* mesh) = 0;
};

struct MeshInfo
{
	std::unordered_map<std::string, ShaderBuffer> vertex_attribute_settings;
	std::shared_ptr<RenderingPipeline> rendering_pipeline;
};

class Mesh
{
public:
	// Structure describing data for a single triangle in the mesh.
	typedef struct Triangle
	{
		size_t indices[3] = { 0, 0, 0 };
	} Triangle;

	Mesh(MeshID id);

	~Mesh();

	static Mesh CreateCube(float side_length);

	void SetVertices(std::vector<glm::vec3> verts);

	void SetTriangles(std::vector<Triangle> tris);

	const std::vector<glm::vec3>& GetVertexPositions() 
	{
		return positions_;
	}

	const std::vector<Triangle>& GetTriangles() 
	{
		return tris_;
	}

	const MeshID& GetInstanceID()
	{
		return id_;
	}

	template<typename T>
	void SetVertexAttributeBuffer(std::string name, std::vector<T> buffer)
	{
		const int type_id = shader::TypeID(buffer[0]);
		const std::vector<char> buffer_data = shader::BufferData(buffer);
		// Check if rendering pipeline actually has a uniform with this name and type.
		if (rendering_pipeline_->HasVertexAttributeWithNameAndType(name, type_id)) {
			uniform_value_map_[name] = { type_id, value_data };
			vertex_attribute_map_[name] = { type_id, num_components, data_type_size, buffer_data };
		}
		else {
			// print warning that a uniform with this name and/or type does not exist for this rendering pipeline.
		}
	}

	std::vector<glm::vec3> GetPositionVertexAttributeBuffer() 
	{

	}

private:
	struct ShaderBuffer {
		int type_id;
		std::vector<char> data;
	};

	MeshID id_;

	// Reserved vertex attributes
	std::vector<glm::vec3> positions_;
	std::vector<glm::vec3> normals_;
	std::vector<glm::vec2> tex_coords_;

	// Un-reserved vertex attributes
	std::unordered_map<std::string, ShaderBuffer> vertex_attribute_map_;
	std::shared_ptr<RenderingPipeline> rendering_pipeline_;
	
	std::vector<Triangle> tris_;

	std::weak_ptr<MeshDelegate> delegate_;
};