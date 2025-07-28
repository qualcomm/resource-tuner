file(GLOB SOURCES "*.cpp")
add_library(SysSignals ${SOURCES})
target_include_directories(SysSignals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SysSignals Server Auxiliary)
