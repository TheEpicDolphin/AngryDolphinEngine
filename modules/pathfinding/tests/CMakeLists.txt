file(GLOB_RECURSE SOURCES *.cpp)

add_executable(pathfinding_tests ${SOURCES})

target_link_libraries(pathfinding_tests PUBLIC gtest)
target_link_libraries(pathfinding_tests PUBLIC pathfinding)

add_test(NAME pathfinding_tests COMMAND pathfinding_tests)