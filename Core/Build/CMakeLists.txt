# CLIENT
file(GLOB SOURCES "../Client/APIs/*.cpp" "../Client/Comm/Socket/*.cpp")
add_library(ClientAPIs ${SOURCES})
set_target_properties(ClientAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(ClientAPIs Auxiliary)
target_include_directories(ClientAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/APIs/Include)
target_include_directories(ClientAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/Comm/Socket/Include)

# EXTENSIONS
file(GLOB SOURCES "../Extensions/*.cpp")
add_library(ExtAPIs ${SOURCES})
set_target_properties(ExtAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(ExtAPIs Auxiliary)
target_include_directories(ExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Extensions/Include)

# FRAMEWORK
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
file(GLOB SOURCES "../Framework/*.cpp")
add_library(Server ${SOURCES})
set_target_properties(Server PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(Server PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework/Include)
target_link_libraries(Server PRIVATE Auxiliary SocketCommServer ExtAPIs ${LIBYAML_LIBRARIES})

# MODULA
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
file(GLOB SOURCES "../Modula/Components/*.cpp" "../Modula/Common/*.cpp" "../Modula/CoreModules/*.cpp")
add_library(Auxiliary ${SOURCES})
set_target_properties(Auxiliary PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(Auxiliary PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_link_libraries(Auxiliary PRIVATE ${LIBYAML_LIBRARIES})

target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Components/Include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Common/Include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/CoreModules/Include)

# SERVER
file(GLOB SOURCES "../Server/Receiver/*.cpp")
add_library(SysRequestReceiver ${SOURCES})
set_target_properties(SysRequestReceiver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(SysRequestReceiver Auxiliary SocketCommServer)
target_include_directories(SysRequestReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Receiver/Include)

file(GLOB SOURCES "../Server/Comm/Socket/*.cpp")
add_library(SocketCommServer ${SOURCES})
set_target_properties(SocketCommServer PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(SocketCommServer Auxiliary)
target_include_directories(SocketCommServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Comm/Socket/Include)
