# CLIENT
file(GLOB SOURCES "../Client/APIs/*.cpp")
add_library(ClientAPIs ${SOURCES})
target_link_libraries(ClientAPIs SocketCommClient Auxiliary)
target_include_directories(ClientAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/APIs/Include)

file(GLOB SOURCES "../Client/Comm/Socket/*.cpp")
add_library(SocketCommClient ${SOURCES})
target_link_libraries(SocketCommClient Auxiliary)
target_include_directories(SocketCommClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/Comm/Socket/Include)

# EXTENSIONS
file(GLOB SOURCES "../Extensions/*.cpp")
add_library(ExtAPIs ${SOURCES})
target_link_libraries(ExtAPIs Auxiliary)
target_include_directories(ExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Extensions/Include)

# FRAMEWORK
file(GLOB SOURCES "../Framework/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
add_library(Server ${SOURCES})

target_include_directories(Server PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework/Include)
target_link_libraries(Server PRIVATE Auxiliary SocketCommServer ExtAPIs SysConfigProps ${YAMLCPP_LIBRARIES})

# MODULA
file(GLOB SOURCES "../Modula/Components/*.cpp" "../Modula/Common/*.cpp" "../Modula/CoreModules/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(Auxiliary ${SOURCES})
target_include_directories(Auxiliary PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_link_libraries(Auxiliary PRIVATE ${YAMLCPP_LIBRARIES})

target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Components/Include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Common/Include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/CoreModules/Include)

# SERVER
file(GLOB SOURCES "../Server/Receiver/*.cpp")
add_library(SysRequestReceiver ${SOURCES})
target_link_libraries(SysRequestReceiver Auxiliary SocketCommServer SysConfigProps)
target_include_directories(SysRequestReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Receiver/Include)

file(GLOB SOURCES "../Server/Socket/*.cpp")
add_library(SocketCommServer ${SOURCES})
target_link_libraries(SocketCommServer Auxiliary)
target_include_directories(SocketCommServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Socket/Include)

file(GLOB SOURCES "../Server/SysConfigs/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(SysConfigProps ${SOURCES})
target_include_directories(SysConfigProps PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(SysConfigProps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/SysConfigs/Include)
target_link_libraries(SysConfigProps Auxiliary)
