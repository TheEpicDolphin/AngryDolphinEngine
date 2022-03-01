file(GLOB SOURCES Source/*.cpp)
add_library(Recast ${SOURCES})

add_library(RecastNavigation::Recast ALIAS Recast)

set(Recast_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")

target_include_directories(Recast PUBLIC
    "$<BUILD_INTERFACE:${Recast_INCLUDE_DIR}>"
)
