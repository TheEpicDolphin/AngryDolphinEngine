file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(test_game ${SOURCES})

target_link_libraries(test_game PUBLIC engine)
target_link_libraries(test_game PUBLIC libglew_static)
target_link_libraries(test_game PUBLIC glfw)