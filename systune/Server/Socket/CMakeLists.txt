file(GLOB SOURCES "*.cpp")
add_library(SystuneCommunicationServer ${SOURCES})
target_link_libraries(SystuneCommunicationServer Auxiliary)
target_include_directories(SystuneCommunicationServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)