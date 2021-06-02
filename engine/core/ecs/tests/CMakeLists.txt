file(GLOB_RECURSE SOURCES *.cpp)

add_executable(ecs_tests ${SOURCES})

target_link_libraries(ecs_tests PUBLIC gtest)
target_link_libraries(ecs_tests PUBLIC utils)

add_test(NAME ecs_tests COMMAND ecs_tests)