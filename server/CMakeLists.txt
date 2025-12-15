# URM Server Endpoint
file(GLOB SOURCES "Receiver/*.cpp"
                  "Comm/Socket/*.cpp")
add_library(UrmReceiver ${SOURCES})
set_target_properties(UrmReceiver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(UrmReceiver PRIVATE UrmAuxUtils)
target_include_directories(UrmReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Socket/Include)
target_include_directories(UrmReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Receiver/Include)
install(TARGETS UrmReceiver LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
