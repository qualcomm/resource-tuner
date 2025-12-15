file(GLOB SOURCES "*.cpp")
add_library(RestuneSignals ${SOURCES})
set_target_properties(RestuneSignals PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneSignals PRIVATE UrmAuxUtils
                                             RestuneExtAPIs
                                             ${LIBYAML_LIBRARIES}
                                             RestuneServer)

target_include_directories(RestuneSignals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
install(TARGETS RestuneSignals DESTINATION ${CMAKE_INSTALL_LIBDIR})
