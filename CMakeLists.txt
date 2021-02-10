cmake_minimum_required(VERSION 2.8)

project(angry_dolphin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
add_subdirectory(3rd_party/glfw)
#include_directories(3rd_party/glfw/include)

find_package(OpenGL REQUIRED)

# GLEW
#add_definitions(-DGLEW_STATIC)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew/glew-2.1.0/build/cmake)
#target_include_directories(glew ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew/glew-2.1.0/include)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew/glew-2.1.0/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew-cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew-cmake/include)

# GLM
set(glm_DIR 3rd_party/glm/cmake/glm) # if necessary
find_package(glm REQUIRED)
add_subdirectory(3rd_party/glm)

add_subdirectory(engine)
add_subdirectory(test_game)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)