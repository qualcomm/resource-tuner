file(GLOB SOURCES "*.cpp")
add_library(ClientAPIs ${SOURCES})
target_link_libraries(ClientAPIs CommunicationClient Auxiliary)
target_include_directories(ClientAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
