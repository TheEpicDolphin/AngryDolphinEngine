#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

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
	Shader(std::vector<char> code);

private:
	ShaderStage stage_;
	std::vector<char> code_;
};

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);