file(GLOB SOURCES "*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
add_library(SysSignals ${SOURCES})
target_include_directories(SysSignals PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
target_include_directories(SysSignals PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_link_libraries(SysSignals PUBLIC Server Auxiliary ExtAPIs ${YAMLCPP_LIBRARIES})
target_include_directories(SysSignals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
