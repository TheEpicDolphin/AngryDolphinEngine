file(GLOB_RECURSE SOURCES *.cpp)

add_executable(tests ${SOURCES})

target_link_libraries(tests PUBLIC gtest)
target_link_libraries(tests PUBLIC utils)

add_test(NAME tests COMMAND tests)