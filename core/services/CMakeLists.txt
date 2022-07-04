file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (services ${SOURCES})

target_link_libraries(services PRIVATE utils)
