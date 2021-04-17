file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

add_library (utils ${libsrc})

set_target_properties(utils PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)