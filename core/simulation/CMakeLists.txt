file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

add_library (simulation ${libsrc})

target_link_libraries(simulation PRIVATE glm::glm)
target_link_libraries(simulation LINK_PUBLIC ecs)

set_target_properties(simulation PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
