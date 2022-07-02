file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library(game ${SOURCES})

target_link_libraries(game PUBLIC libglew_static)
target_link_libraries(game PUBLIC glfw)

target_link_libraries(game PUBLIC scene)
target_link_libraries(game PUBLIC graphics)

set_target_properties(game PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
