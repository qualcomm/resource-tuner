cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)

project("Resource Tuner")

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

# Enable Debug. should be removed in production.
# set(CMAKE_BUILD_TYPE Debug)

# Always build shared libs
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Optional Modules
option(BUILD_SIGNALS "Signals" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "CLI" OFF)

# Mandatory Builds
add_subdirectory(../Core/Build ${CMAKE_BINARY_DIR}/CoreBuild)

if(BUILD_SIGNALS)
  add_subdirectory(../Signals/Build ${CMAKE_BINARY_DIR}/SignalsBuild)
endif()

if(BUILD_TESTS)
  add_subdirectory(../Tests/Unit/Build ${CMAKE_BINARY_DIR}/TestBuild)
endif()

if(BUILD_CLI)
  add_executable(resource_tuner_cli
    ../Core/Client/APIs/ResourceTunerAPIs.cpp
    ../Core/Client/ClientCLI.cpp
  )
  target_link_libraries(resource_tuner_cli SocketCommClient ClientAPIs)
  install(TARGETS resource_tuner_cli RUNTIME DESTINATION bin)
endif()

if(BUILD_SIGNALS)
  add_executable(resource_tuner ../ServerMain.cpp ../Signals/SignalInit.cpp)
  target_include_directories(resource_tuner PRIVATE ${YAMLCPP_INCLUDE_DIRS})
  target_include_directories(resource_tuner PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
  target_link_libraries(resource_tuner PUBLIC SysSignals)
  target_link_libraries(resource_tuner PUBLIC ExtAPIs)
else()
  add_executable(resource_tuner ../ServerMain.cpp)
  target_include_directories(resource_tuner PRIVATE ${YAMLCPP_INCLUDE_DIRS})
  target_include_directories(resource_tuner PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
  target_link_libraries(resource_tuner PUBLIC ExtAPIs)
endif()

target_link_libraries(resource_tuner PUBLIC Server Auxiliary SysRequestReceiver)

# Test Executable
if(BUILD_TESTS)
  add_executable(resource_tuner_tests ../Tests/System/SystemWideTests.cpp)
  target_link_libraries(resource_tuner_tests PUBLIC Auxiliary ClientAPIs)
  install(TARGETS resource_tuner_tests RUNTIME DESTINATION bin)
endif()

# Install libs
install(TARGETS resource_tuner RUNTIME DESTINATION bin)
install(TARGETS Auxiliary SocketCommClient SocketCommServer
        ClientAPIs Server ExtAPIs
        SysRequestReceiver LIBRARY DESTINATION lib)
if(BUILD_SIGNALS)
  install(TARGETS SysSignals DESTINATION lib)
endif()

# Install headers
install(DIRECTORY ../Core/Client/APIs/Include/ DESTINATION include/ResourceTuner)
install(DIRECTORY ../Core/Extensions/Include/ DESTINATION include/ResourceTuner)

# Install core config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/../Core/Configs/
  DESTINATION  /etc/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install Signal config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/../Signals/Configs/
  DESTINATION /etc/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install Test Configs, if BUILD_TEST is enabled
if(BUILD_TESTS)
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/../Tests/Configs/
    DESTINATION /etc/resource-tuner/tests/Configs/
    FILES_MATCHING
    PATTERN "*.yaml"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()

# Install Custom Resource Nodes, if BUILD_TEST is enabled
if(BUILD_TESTS)
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/../Tests/Configs/ResourceSysFsNodes/
    DESTINATION /etc/resource-tuner/tests/Configs/ResourceSysFsNodes/
    PATTERN "*"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()
