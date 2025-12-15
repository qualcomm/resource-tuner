# EXTENSIONS
file(GLOB SOURCES "Extensions/*.cpp")
add_library(RestuneExtAPIs ${SOURCES})
set_target_properties(RestuneExtAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneExtAPIs PRIVATE UrmAuxUtils)
target_include_directories(RestuneExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Extensions/Include)
install(TARGETS RestuneExtAPIs LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Extensions/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)

# FRAMEWORK
file(GLOB SOURCES "Framework/*.cpp")
add_library(RestuneCore ${SOURCES})
set_target_properties(RestuneCore PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneCore PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(RestuneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Framework/Include)
target_link_libraries(RestuneCore PRIVATE UrmAuxUtils
                                            RestuneExtAPIs
                                            ${LIBYAML_LIBRARIES})

install(TARGETS RestuneCore LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
