file(GLOB SOURCES "*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(SysConfigProps ${SOURCES})
target_include_directories(SysConfigProps PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(SysConfigProps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(SysConfigProps Auxiliary)
