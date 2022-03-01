cmake_minimum_required(VERSION 3.0)

include(GNUInstallDirs)

add_subdirectory(Detour)
add_subdirectory(Recast)

add_library(RecastNavigation INTERFACE)
