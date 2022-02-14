file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (resource_manager ${SOURCES})

target_link_libraries(resource_manager PUBLIC config)

set_target_properties(resource_manager PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)