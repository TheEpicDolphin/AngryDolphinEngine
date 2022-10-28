file(GLOB_RECURSE SOURCES *.cpp)

add_executable(graphics_tests ${SOURCES})

target_link_libraries(graphics_tests PRIVATE graphics)
#target_link_libraries(graphics_tests PRIVATE serialize)
target_link_libraries(graphics_tests PRIVATE gtest)

add_test(NAME graphics_tests COMMAND graphics_tests)
