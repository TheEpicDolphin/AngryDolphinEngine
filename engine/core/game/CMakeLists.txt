file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

add_library(game ${libsrc})

target_link_libraries(game LINK_PUBLIC ecs)

set_target_properties(game PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
