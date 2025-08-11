file(GLOB SOURCES "Components/*.cpp" "Common/*.cpp" "CoreModules/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

add_library(Auxiliary ${SOURCES})
target_include_directories(Auxiliary PRIVATE ${YAMLCPP_INCLUDE_DIRS})
target_link_libraries(Auxiliary PRIVATE ${YAMLCPP_LIBRARIES})

target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Components/include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/CoreModules/include)
