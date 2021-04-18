add_subdirectory(ecs)
add_subdirectory(graphics)
add_subdirectory(game)
add_subdirectory(simulation)
add_subdirectory(transform)
add_subdirectory(object)
add_subdirectory(utils)

add_library(core INTERFACE)

target_link_libraries(core INTERFACE ecs)
target_link_libraries(core INTERFACE graphics)
target_link_libraries(core INTERFACE game)
target_link_libraries(core INTERFACE simulation)
target_link_libraries(core INTERFACE transform)
target_link_libraries(core INTERFACE object)
target_link_libraries(core INTERFACE utils)

# This means that to access a header file in ecs, you would #include<core/ecs/example_header.h>
target_include_directories(ecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)