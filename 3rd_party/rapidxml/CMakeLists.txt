add_library (rapidxml INTERFACE)
target_include_directories(rapidxml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(rapidxml PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)