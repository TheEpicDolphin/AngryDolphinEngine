file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (my_scenes ${SOURCES})

target_link_libraries(my_scenes PRIVATE scene)
target_link_libraries(my_scenes PRIVATE graphics)
target_link_libraries(my_scenes PRIVATE simulation)
