file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (object ${SOURCES})

set_target_properties(object PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
