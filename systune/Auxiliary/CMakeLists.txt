file(GLOB SOURCES "Components/*.cpp" "Common/*.cpp" "CoreModules/*.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

add_library(Auxiliary ${SOURCES})
target_include_directories(Auxiliary PRIVATE ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(Auxiliary PRIVATE ${JSONCPP_LIBRARIES})

target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Components/include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Common/include)
target_include_directories(Auxiliary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/CoreModules/include)
