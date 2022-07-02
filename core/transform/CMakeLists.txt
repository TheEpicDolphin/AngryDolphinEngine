file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (transform ${SOURCES})

target_link_libraries(transform PRIVATE glm::glm)
