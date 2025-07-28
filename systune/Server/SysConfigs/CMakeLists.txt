file(GLOB SOURCES "*.cpp")
add_library(SysConfigProps ${SOURCES})
target_include_directories(SysConfigProps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SysConfigProps Auxiliary)
