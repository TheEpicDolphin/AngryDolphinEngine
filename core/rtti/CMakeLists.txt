file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (rtti ${SOURCES})

target_link_libraries(rtti PRIVATE utils)
