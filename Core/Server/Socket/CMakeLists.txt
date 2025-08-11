file(GLOB SOURCES "*.cpp")
add_library(ResourceTunerCommunicationServer ${SOURCES})
target_link_libraries(ResourceTunerCommunicationServer Auxiliary)
target_include_directories(ResourceTunerCommunicationServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)