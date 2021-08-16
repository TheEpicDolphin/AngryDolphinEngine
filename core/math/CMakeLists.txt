file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (math ${SOURCES})

target_link_libraries(math PUBLIC glm)

set_target_properties(math PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
