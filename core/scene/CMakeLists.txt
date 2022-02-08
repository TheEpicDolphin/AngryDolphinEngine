file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (scene ${SOURCES})

target_link_libraries(scene PUBLIC glm::glm)
target_link_libraries(scene PUBLIC ecs)
target_link_libraries(scene PUBLIC utils)
target_link_libraries(scene PUBLIC transform)
target_link_libraries(scene PUBLIC graphics)

set_target_properties(scene PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
