file(GLOB SOURCES "../../Extensions/*.cpp")
add_library(ExtAPIs ${SOURCES})
target_link_libraries(ExtAPIs Auxiliary)
target_include_directories(ExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Extensions/Include)
