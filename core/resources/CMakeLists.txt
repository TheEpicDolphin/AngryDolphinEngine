file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (resources ${SOURCES})

set_target_properties(resources PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)