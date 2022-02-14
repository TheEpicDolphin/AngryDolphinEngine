file(GLOB_RECURSE SOURCES *.h *.cpp *.hpp *.c *.cc)

add_library (graphics ${SOURCES})

target_link_libraries(graphics PUBLIC libglew_static)
target_link_libraries(graphics PUBLIC glm::glm)
target_link_libraries(graphics PUBLIC ecs)
target_link_libraries(graphics PUBLIC glfw)
#target_link_libraries(graphics PUBLIC rapidjson)
target_link_libraries(graphics PUBLIC serialize)
target_link_libraries(graphics PUBLIC utils)
target_link_libraries(graphics PUBLIC geometry)
target_link_libraries(graphics PUBLIC resource_manager)

set_target_properties(graphics PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
