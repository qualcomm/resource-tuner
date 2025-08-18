file(GLOB SOURCES "../../Framework/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
add_library(Server ${SOURCES})

target_include_directories(Server PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Framework/Include)
target_link_libraries(Server PRIVATE Auxiliary SocketCommServer ExtAPIs SysConfigProps ${YAMLCPP_LIBRARIES})
