file(GLOB_RECURSE SOURCES *.cpp)

add_executable(utils_tests ${SOURCES})

target_link_libraries(utils_tests PUBLIC gtest)

add_test(NAME utils_tests COMMAND utils_tests)