file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (transform ${SOURCES})

target_link_libraries(transform glm::glm)
target_link_libraries(transform LINK_PUBLIC ecs)

set_target_properties(transform PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
