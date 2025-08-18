file(GLOB SOURCES "../../Server/Receiver/*.cpp")
add_library(SysRequestReceiver ${SOURCES})
target_link_libraries(SysRequestReceiver Auxiliary SocketCommServer SysConfigProps)
target_include_directories(SysRequestReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/Receiver/Include)

file(GLOB SOURCES "../../Server/Socket/*.cpp")
add_library(SocketCommServer ${SOURCES})
target_link_libraries(SocketCommServer Auxiliary)
target_include_directories(SocketCommServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/Socket/Include)

file(GLOB SOURCES "../../Server/SysConfigs/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(SysConfigProps ${SOURCES})
target_include_directories(SysConfigProps PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(SysConfigProps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/SysConfigs/Include)
target_link_libraries(SysConfigProps Auxiliary)
