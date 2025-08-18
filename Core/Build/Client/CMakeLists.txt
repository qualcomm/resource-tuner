file(GLOB SOURCES "../../Client/APIs/*.cpp")
add_library(ClientAPIs ${SOURCES})
target_link_libraries(ClientAPIs SocketCommClient Auxiliary)
target_include_directories(ClientAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Client/APIs/Include)

file(GLOB SOURCES "../../Client/Comm/Socket/*.cpp")
add_library(SocketCommClient ${SOURCES})
target_link_libraries(SocketCommClient Auxiliary)
target_include_directories(SocketCommClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Client/Comm/Socket/Include)
