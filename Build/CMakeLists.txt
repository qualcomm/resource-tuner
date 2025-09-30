cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)

project("Resource Tuner")

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

# Enable Debug. should be removed in production.
# set(CMAKE_BUILD_TYPE Debug)

# Always build shared libs
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Optional Modules
option(BUILD_SIGNALS "Signals" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "CLI" OFF)

# Mandatory Builds
add_subdirectory(${CMAKE_SOURCE_DIR}/../Core/Build ${CMAKE_BINARY_DIR}/CoreBuild)

if(BUILD_SIGNALS)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../Signals/Build ${CMAKE_BINARY_DIR}/SignalsBuild)
endif()

if(BUILD_TESTS)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../Tests/Build ${CMAKE_BINARY_DIR}/TestBuild)
endif()

if(BUILD_CLI)
  add_executable(resource_tuner_cli
    ${CMAKE_SOURCE_DIR}/../Core/Client/APIs/ResourceTunerAPIs.cpp
    ${CMAKE_SOURCE_DIR}/../Core/Client/ClientCLI.cpp
  )
  target_link_libraries(resource_tuner_cli ClientAPIs)
  install(TARGETS resource_tuner_cli RUNTIME DESTINATION bin)
endif()

if(BUILD_SIGNALS)
  add_executable(resource_tuner ${CMAKE_SOURCE_DIR}/../ServerMain.cpp ${CMAKE_SOURCE_DIR}/../Signals/SignalInit.cpp)
  target_link_libraries(resource_tuner PUBLIC SysSignals)
else()
  add_executable(resource_tuner ${CMAKE_SOURCE_DIR}/../ServerMain.cpp)
endif()

target_include_directories(resource_tuner PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(resource_tuner PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)

target_link_libraries(resource_tuner PUBLIC Server Auxiliary SysRequestReceiver)
target_link_libraries(resource_tuner PUBLIC ExtAPIs)

# Install libs
install(TARGETS resource_tuner RUNTIME DESTINATION bin)
install(TARGETS Auxiliary SocketCommServer
                ClientAPIs Server ExtAPIs
                SysRequestReceiver LIBRARY DESTINATION lib)

if(BUILD_TESTS)
  install(TARGETS TestSetupUtils DESTINATION lib)
endif()

if(BUILD_SIGNALS)
  install(TARGETS SysSignals DESTINATION lib)
endif()

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/../Core/Client/APIs/Include/ DESTINATION include/ResourceTuner)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/../Core/Extensions/Include/ DESTINATION include/ResourceTuner)
install(FILES ${CMAKE_SOURCE_DIR}/../Core/Modula/Common/Include/Common.h DESTINATION include/ResourceTuner)

# Install core config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/../Core/Configs/
  DESTINATION  etc/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install Signal config files
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/../Signals/Configs/
  DESTINATION etc/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install Test Configs, if BUILD_TEST is enabled
if(BUILD_TESTS)
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/../Tests/Configs/
    DESTINATION etc/resource-tuner/custom/
    FILES_MATCHING
    PATTERN "*.yaml"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()

# Install Custom Resource Nodes, if BUILD_TEST is enabled
if(BUILD_TESTS)
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/../Tests/Configs/ResourceSysFsNodes/
    DESTINATION etc/resource-tuner/tests/Configs/ResourceSysFsNodes/
    FILES_MATCHING
    PATTERN "*"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif()

# Install service file
install(FILES ${CMAKE_SOURCE_DIR}/../Services/resource-tuner.service DESTINATION lib/systemd/system/)

