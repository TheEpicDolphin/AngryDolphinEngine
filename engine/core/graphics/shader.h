#pragma once

#include <GL/glew.h>

enum ShaderType { 
	ShaderTypeVertex, 
	ShaderTypeFragment,
	ShaderTypeGeometry,
	ShaderTypeCompute
};

struct Shader 
{
	ShaderType type;
	bool is_spirv;
	char* code;
};

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);