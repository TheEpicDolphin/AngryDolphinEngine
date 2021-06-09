
#include "glfw_window_wrapper.h"

GLFWWindowWrapper::GLFWWindowWrapper(int width, int height) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
        
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window_ = glfwCreateWindow(width, height, "Test GLFW Window", NULL, NULL);
    if (!window_)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void GLFWWindowWrapper::Render() {
    if (!glfwWindowShouldClose(window_)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void GLFWWindowWrapper::Destroy() {
    glfwDestroyWindow(window_);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

int GLFWWindowWrapper::FrameBufferSize(int *width, int *height) {
    glfwGetFramebufferSize(window_, width, height);
}