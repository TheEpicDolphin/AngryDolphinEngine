file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (utils ${SOURCES})

target_link_libraries(utils PUBLIC gtest)
target_link_libraries(utils PRIVATE glm::glm)

add_subdirectory(tests)

set_target_properties(utils PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)