add_library (definitions INTERFACE)

target_link_libraries(definitions INTERFACE geometry)
target_link_libraries(definitions INTERFACE glm)