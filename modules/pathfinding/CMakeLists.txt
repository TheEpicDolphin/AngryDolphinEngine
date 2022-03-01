file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (pathfinding ${SOURCES})

target_link_libraries(pathfinding PUBLIC Recast)
target_link_libraries(pathfinding PUBLIC Detour)

add_subdirectory(tests)

set_target_properties(pathfinding PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)