file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(example_game ${SOURCES})

target_link_libraries(example_game PRIVATE game)
target_link_libraries(example_game PRIVATE my_scenes)
