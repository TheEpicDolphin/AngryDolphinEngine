file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (serialize ${SOURCES})

target_link_libraries(serialize PUBLIC gtest)

add_subdirectory(serialize)

set_target_properties(serialize PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)