#pragma once

#include <GL/glew.h>
#include <vector>

enum ShaderType { 
	ShaderTypeVertex, 
	ShaderTypeFragment,
	ShaderTypeGeometry,
	ShaderTypeCompute
};

struct Shader 
{
	ShaderType type;
	std::vector<char> code;
};

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);