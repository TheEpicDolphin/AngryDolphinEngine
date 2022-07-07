file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (resource_manager ${SOURCES})

target_link_libraries(resource_manager PRIVATE config)
