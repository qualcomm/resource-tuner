cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)

project("Resource Tuner")

include(GNUInstallDirs)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

# Enable Debug. should be removed in production.
# set(CMAKE_BUILD_TYPE Debug)

# Always build shared libs
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Optional Modules
option(BUILD_SIGNALS "Signals" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "CLI" OFF)
option(BUILD_STATE_DETECTOR "Display Detector" OFF)

# Mandatory Builds
add_subdirectory(${CMAKE_SOURCE_DIR}/Core/Build ${CMAKE_BINARY_DIR}/CoreBuild)

if(BUILD_SIGNALS)
  add_subdirectory(${CMAKE_SOURCE_DIR}/Signals/Build ${CMAKE_BINARY_DIR}/SignalsBuild)
endif()

if(BUILD_TESTS)
  add_subdirectory(${CMAKE_SOURCE_DIR}/Tests/Build ${CMAKE_BINARY_DIR}/TestBuild)
endif()

if(BUILD_CLI)
  add_executable(resource_tuner_cli
    ${CMAKE_SOURCE_DIR}/Core/Client/APIs/ResourceTunerAPIs.cpp
    ${CMAKE_SOURCE_DIR}/Core/Client/ClientCLI.cpp
  )
  target_link_libraries(resource_tuner_cli ClientAPIs)
  install(TARGETS resource_tuner_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

set(SERVER_EXECUTABLE "")
list(APPEND SERVER_EXECUTABLE ${CMAKE_SOURCE_DIR}/ServerMain.cpp)

set(SERVER_LIBS "")
list(APPEND SERVER_LIBS
  Server
  Auxiliary
  SysRequestReceiver
  ExtAPIs
)

if(BUILD_STATE_DETECTOR)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_SOURCE_DIR}/StateDetector.cpp)
  list(APPEND SERVER_LIBS systemd)
endif()

if(BUILD_SIGNALS)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_SOURCE_DIR}/Signals/SignalInit.cpp)
  list(APPEND SERVER_LIBS SysSignals)
endif()

add_executable(resource_tuner ${SERVER_EXECUTABLE})
target_link_libraries(resource_tuner PUBLIC ${SERVER_LIBS})
target_include_directories(resource_tuner PRIVATE ${LIBYAML_INCLUDE_DIRS})

# Install bins
install(TARGETS resource_tuner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install libs
install(TARGETS Auxiliary SocketCommServer
                ClientAPIs Server ExtAPIs
                SysRequestReceiver LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(BUILD_SIGNALS)
  install(TARGETS SysSignals DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Client/APIs/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Extensions/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(FILES ${CMAKE_SOURCE_DIR}/Core/Modula/Common/Include/Common.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)

# Install service file
install(FILES ${CMAKE_SOURCE_DIR}/Services/resource-tuner.service DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system/)

# Install core config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Configs/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install Signal config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/Signals/Configs/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

if(BUILD_TESTS)
  # Install Test Configs
  file(GLOB testConfigs "${CMAKE_SOURCE_DIR}/Tests/Configs/*.yaml")
  install(
    FILES ${testConfigs}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/custom/
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )

  # Install Custom Resource Nodes
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/Tests/Configs/ResourceSysFsNodes/
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/tests/Configs/ResourceSysFsNodes/
    FILES_MATCHING
    PATTERN "*.txt"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()
