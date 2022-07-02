file(GLOB_RECURSE SOURCES *.cpp)

add_executable(serialize_tests ${SOURCES})

target_link_libraries(serialize_tests PUBLIC serialize)
target_link_libraries(serialize_tests PUBLIC gtest)

add_test(NAME serialize_tests COMMAND serialize_tests)