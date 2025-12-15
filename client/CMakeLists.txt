# Install URM Client Lib
file(GLOB SOURCES "APIs/*.cpp"
                  "Comm/Socket/*.cpp")
add_library(UrmClient ${SOURCES})
set_target_properties(UrmClient PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(UrmClient PRIVATE UrmAuxUtils)
target_include_directories(UrmClient PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/APIs/Include)
target_include_directories(UrmClient PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Comm/Socket/Include)
install(TARGETS UrmClient LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# URM CLI Client
add_executable(urmClientCmd
  ${CMAKE_CURRENT_SOURCE_DIR}/APIs/ResourceTunerAPIs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ClientCLI.cpp
)
target_link_libraries(urmClientCmd PRIVATE UrmAuxUtils)
target_link_libraries(urmClientCmd PRIVATE UrmClient)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/APIs/Include)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/client/Comm/Socket/Include)
install(TARGETS urmClientCmd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/APIs/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
