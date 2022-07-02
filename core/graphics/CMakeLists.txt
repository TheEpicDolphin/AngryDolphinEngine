file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (graphics ${SOURCES})

target_link_libraries(graphics PRIVATE libglew_static)
target_link_libraries(graphics PRIVATE glm::glm)
target_link_libraries(graphics PRIVATE glfw)
target_link_libraries(graphics PRIVATE rapidxml)

target_link_libraries(graphics PRIVATE definitions)
target_link_libraries(graphics PRIVATE ecs)
target_link_libraries(graphics PRIVATE serialize)
target_link_libraries(graphics PRIVATE utils)
target_link_libraries(graphics PRIVATE geometry)
target_link_libraries(graphics PRIVATE resource_manager)
target_link_libraries(graphics PRIVATE scene)
target_link_libraries(graphics PRIVATE transform)
