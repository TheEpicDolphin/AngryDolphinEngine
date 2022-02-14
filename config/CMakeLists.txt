file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (config ${SOURCES})
