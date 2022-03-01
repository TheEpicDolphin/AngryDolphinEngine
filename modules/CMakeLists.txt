add_subdirectory(pathfinding)

add_library(modules INTERFACE)

target_link_libraries(modules INTERFACE pathfinding)

# This means that to access a header file in ecs, you would #include<modules/pathfinding/example_header.h>
target_include_directories(pathfinding PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)