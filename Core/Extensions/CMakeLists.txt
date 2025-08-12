file(GLOB SOURCES "*.cpp")
add_library(ExtAPIs ${SOURCES})
target_link_libraries(ExtAPIs Auxiliary)
target_include_directories(ExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(ExtensionHeaders INTERFACE)
target_include_directories(ExtensionHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)