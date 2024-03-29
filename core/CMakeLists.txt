add_subdirectory(definitions)
add_subdirectory(ecs)
add_subdirectory(graphics)
add_subdirectory(game)
add_subdirectory(scene)
#add_subdirectory(serialize)
add_subdirectory(simulation)
add_subdirectory(utils)
add_subdirectory(geometry)
add_subdirectory(transform)
add_subdirectory(resource_manager)
add_subdirectory(services)
add_subdirectory(rtti)

add_library(core INTERFACE)

target_link_libraries(core INTERFACE definitions)
target_link_libraries(core INTERFACE ecs)
target_link_libraries(core INTERFACE graphics)
target_link_libraries(core INTERFACE game)
target_link_libraries(core INTERFACE scene)
#target_link_libraries(core INTERFACE serialize)
target_link_libraries(core INTERFACE simulation)
target_link_libraries(core INTERFACE utils)
target_link_libraries(core INTERFACE geometry)
target_link_libraries(core INTERFACE transform)
target_link_libraries(core INTERFACE resource_manager)
target_link_libraries(core INTERFACE services)
target_link_libraries(core INTERFACE rtti)

# This means that to access a header file in ecs, you would #include<core/ecs/example_header.h>
target_include_directories(definitions INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(ecs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
#target_include_directories(serialize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(geometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(transform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(resource_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(rtti PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)