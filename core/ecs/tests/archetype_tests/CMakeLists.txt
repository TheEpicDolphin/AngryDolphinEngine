file(GLOB_RECURSE SOURCES *.cpp)

add_executable(archetype_test ${SOURCES})

target_link_libraries(archetype_test PUBLIC gtest)
target_link_libraries(archetype_test PUBLIC utils)

add_test(NAME archetype_test COMMAND archetype_test)