file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (geometry ${SOURCES})

target_link_libraries(geometry PRIVATE glm::glm)
target_link_libraries(geometry PRIVATE transform)

set_target_properties(geometry PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)