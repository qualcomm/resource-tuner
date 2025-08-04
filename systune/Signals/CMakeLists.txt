file(GLOB SOURCES "*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
add_library(SysSignals ${SOURCES})
target_include_directories(SysSignals PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/include)
target_include_directories(SysSignals PRIVATE ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(SysSignals PUBLIC Server Auxiliary ${JSONCPP_LIBRARIES})
target_include_directories(SysSignals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
