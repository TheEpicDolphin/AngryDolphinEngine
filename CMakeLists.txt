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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew-cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glew-cmake/include)

# GLM
add_subdirectory(3rd_party/glm)

# Google test
enable_testing()
# This prevents "error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MTd_StaticDebug' doesn't match value 'MDd_DynamicDebug' in main.obj"
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(3rd_party/googletest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/googletest/googletest/include)

# RapidJSON
add_subdirectory(3rd_party/rapidjson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/rapidjson/include)

add_subdirectory(core)

add_library(angery_dolphine_engine INTERFACE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in.txt ${CMAKE_CURRENT_SOURCE_DIR}/config/generated/config.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config/)
