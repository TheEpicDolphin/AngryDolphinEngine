#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "uniform.h"

enum ShaderStage { 
	ShaderStageVertex = 0, 
	ShaderStageGeometry,
	ShaderStageFragment,
	ShaderStageCompute
};

class Shader 
{
public:
	Shader(std::vector<Uniform> uniforms, std::vector<char> code);

	const Uniform& UniformForName(std::string name);
private:
	ShaderStage stage_;
	std::vector<char> code_;
	std::unordered_map<std::string, Uniform> uniform_map_;
};

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);