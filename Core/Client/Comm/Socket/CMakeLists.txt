file(GLOB SOURCES "*.cpp")
add_library(CommunicationClient ${SOURCES})
target_link_libraries(CommunicationClient Auxiliary)
target_include_directories(CommunicationClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)