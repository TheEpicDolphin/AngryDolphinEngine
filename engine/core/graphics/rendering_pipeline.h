#pragma once

#include <vector>
#include <unordered_map>
#include "shader/shader.h"

typedef UID PipelineID;

struct RenderingPipelineDelegate
{
	virtual void PipelineDidDestruct(RenderingPipeline* pipeline) = 0;
};

struct RenderingPipelineInfo
{

};

struct UniformInfo {
	std::string name;
	int typeId;
};

struct VertexAttributeInfo {
	std::string name;
	// Type of the data
	int typeId;
	// Location in the shader
	int location;
	// Number of 
	int dimension;
	// Format
	int format;
};

// This is equivalent to a "pipeline" in Vulkan and a "program" in OpenGL.
class RenderingPipeline 
{
public:
	RenderingPipeline();

	RenderingPipeline(PipelineID pipeline_id, std::vector<UniformInfo> uniforms, std::vector<Shader> shader_stages);

	const PipelineID& GetInstanceID();

	bool HasUniformWithNameAndType(std::string name, int typeId);

	bool HasVertexAttributeWithNameAndType(std::string name, int typeId);

	const std::vector<Shader>& ShaderStages();

	const std::vector<VertexAttributeInfo>& VertexAttributes();

private:
	
	PipelineID id_;
	std::vector<VertexAttributeInfo> vertex_attributes_;
	std::unordered_map<std::string, UniformInfo> uniform_info_map_;
	std::unordered_map<std::string, VertexAttributeInfo> vertex_attributes_info_map_;
	std::vector<Shader> shader_stages_;
};