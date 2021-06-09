#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "render_context.h"

class GLFWWindowWrapper : IRenderContext {
	GLFWWindowWrapper(int width, int height);

private:
	GLFWwindow window_;
};