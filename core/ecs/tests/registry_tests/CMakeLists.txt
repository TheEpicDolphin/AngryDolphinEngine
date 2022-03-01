file(GLOB_RECURSE SOURCES *.cpp)

add_executable(ecs_test ${SOURCES})

target_link_libraries(ecs_test PUBLIC gtest)
target_link_libraries(ecs_test PUBLIC utils)

add_test(NAME ecs_test COMMAND ecs_test)