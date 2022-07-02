file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (scene ${SOURCES})

target_link_libraries(scene PRIVATE definitions)
target_link_libraries(scene PRIVATE glm::glm)
target_link_libraries(scene PRIVATE ecs)
target_link_libraries(scene PRIVATE utils)
target_link_libraries(scene PRIVATE transform)
target_link_libraries(scene PRIVATE serialize)
