# EXTENSIONS
file(GLOB SOURCES "Extensions/*.cpp")
add_library(RestuneExtAPIs ${SOURCES})
set_target_properties(RestuneExtAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneExtAPIs PRIVATE UrmAuxUtils)
target_include_directories(RestuneExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Extensions/Include)
install(TARGETS RestuneExtAPIs LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# FRAMEWORK
file(GLOB SOURCES "Framework/*.cpp")
add_library(RestuneServer ${SOURCES})
set_target_properties(RestuneServer PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneServer PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(RestuneServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Framework/Include)
target_link_libraries(RestuneServer PRIVATE UrmAuxUtils
                                           RestuneSockEnd
                                           RestuneExtAPIs
                                           ${LIBYAML_LIBRARIES})

install(TARGETS RestuneServer LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# SERVER / RECEIVER ENDPOINT
file(GLOB SOURCES "Server/Receiver/*.cpp")
add_library(RestuneReceiver ${SOURCES})
set_target_properties(RestuneReceiver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneReceiver PRIVATE UrmAuxUtils)
target_link_libraries(RestuneReceiver PUBLIC RestuneSockEnd)
target_include_directories(RestuneReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Server/Receiver/Include)
install(TARGETS RestuneReceiver LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

file(GLOB SOURCES "Server/Socket/*.cpp")
add_library(RestuneSockEnd ${SOURCES})
set_target_properties(RestuneSockEnd PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneSockEnd PRIVATE UrmAuxUtils)
target_include_directories(RestuneSockEnd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Server/Socket/Include)
install(TARGETS RestuneSockEnd LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
