file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (simulation ${SOURCES})

target_link_libraries(simulation PRIVATE glm::glm)
target_link_libraries(simulation PRIVATE ecs)
target_link_libraries(simulation PRIVATE scene)
target_link_libraries(simulation PRIVATE transform)
