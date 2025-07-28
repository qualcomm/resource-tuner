file(GLOB SOURCES "*.cpp")
add_library(Server ${SOURCES})
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Server Auxiliary SystuneCommunicationServer ExtAPIs ClientAPIs SysConfigProps)
