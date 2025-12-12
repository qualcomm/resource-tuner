# Optional Modules
option(BUILD_SIGNALS "Signals" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "CLI" OFF)
option(BUILD_STATE_DETECTOR "Display Detector" OFF)

# Mandatory Builds
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Build ${CMAKE_BINARY_DIR}/CoreBuild)

if(BUILD_SIGNALS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Signals/Build ${CMAKE_BINARY_DIR}/SignalsBuild)
endif()

if(BUILD_TESTS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests/Build ${CMAKE_BINARY_DIR}/TestBuild)
endif()

if(BUILD_CLI)
  add_executable(resource_tuner_cli
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Client/APIs/ResourceTunerAPIs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Client/ClientCLI.cpp
  )
  target_link_libraries(resource_tuner_cli RestuneClient)
  install(TARGETS resource_tuner_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

set(SERVER_EXECUTABLE "")
list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/ServerMain.cpp)

set(SERVER_LIBS "")
list(APPEND SERVER_LIBS
  RestuneServer
  RestuneAuxUtils
  RestuneReceiver
  RestuneExtAPIs
)

if(BUILD_STATE_DETECTOR)
  pkg_check_modules(SYSTEMD REQUIRED libsystemd)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/StateDetector.cpp)
  list(APPEND SERVER_LIBS ${SYSTEMD_LIBRARIES})
endif()

if(BUILD_SIGNALS)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/Signals/SignalInit.cpp)
  list(APPEND SERVER_LIBS RestuneSignals)
endif()

add_executable(resource_tuner ${SERVER_EXECUTABLE})
target_link_libraries(resource_tuner PUBLIC ${SERVER_LIBS})
target_include_directories(resource_tuner PRIVATE ${LIBYAML_INCLUDE_DIRS})

# Install bins
install(TARGETS resource_tuner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install libs
install(TARGETS RestuneAuxUtils RestuneSockEnd
                RestuneClient RestuneServer RestuneExtAPIs
                RestuneReceiver LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(BUILD_SIGNALS)
  install(TARGETS RestuneSignals DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Core/Client/APIs/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Core/Extensions/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Core/Modula/Common/Include/Common.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)

# Install core config files
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../configs/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

if(BUILD_TESTS)
  # Install Test Configs
  file(GLOB testConfigs "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Configs/*.yaml")
  install(
    FILES ${testConfigs}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/tests/configs/
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )

  # Install Custom Resource Nodes
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Tests/Configs/ResourceSysFsNodes/
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/tests/nodes/
    FILES_MATCHING
    PATTERN "*.txt"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()
