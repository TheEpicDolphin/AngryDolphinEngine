file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (ecs ${SOURCES})

target_link_libraries(ecs PUBLIC config)

add_subdirectory(tests)

set_target_properties(ecs PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
