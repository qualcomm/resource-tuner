file(GLOB SOURCES "*.cpp")
add_library(SysRequestReceiver ${SOURCES})
target_link_libraries(SysRequestReceiver Auxiliary SystuneCommunicationServer SysConfigProps)
target_include_directories(SysRequestReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
