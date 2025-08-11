file(GLOB SOURCES "*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
add_library(Server ${SOURCES})

target_include_directories(Server PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Server PRIVATE Auxiliary SystuneCommunicationServer ExtAPIs ClientAPIs SysConfigProps ${YAMLCPP_LIBRARIES})
