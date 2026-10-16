# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

file(GLOB_RECURSE app_sources
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../lib/*.c"
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/../include"
        "${CMAKE_CURRENT_LIST_DIR}/../lib"
        "${CMAKE_CURRENT_LIST_DIR}/../lib/oled"
    REQUIRES driver
)
