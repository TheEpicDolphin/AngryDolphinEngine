add_subdirectory(ecs)
add_subdirectory(graphics)

add_library(core INTERFACE)

target_link_libraries(core INTERFACE ecs)
target_link_libraries(core INTERFACE graphics)

# This means that to access a header file in ecs, you would #include<core/ecs/example_header.h>
target_include_directories(ecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)