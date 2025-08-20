cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)
# for gdb. can be removed in production.
set(CMAKE_BUILD_TYPE Debug)

project("Resource Tuner")

option(BUILD_DOC "Build documentation" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" ON)
option(BUILD_SIGNALS "Build SysSignals" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "Build CLI" OFF)

#GoogleTest dependency (Update the hash often)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e90fe2485641bab0d6af4500192dc503384950d1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(yaml-cpp)

# Enable Doxygen documentation build
# first we can indicate the documentation build as an option and set it to OFF by default
if(BUILD_DOC)
# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Docs/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")
    message("COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
endif()

# Mandatory Builds
add_subdirectory(Core/Build)

# Optional builds
if(BUILD_SIGNALS)
  add_subdirectory(Signals/Build)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(Tests/Unit/Build)
endif()

if(BUILD_CLI)
  add_executable(resource_tuner_cli
    ./Core/Client/APIs/ResourceTunerAPIs.cpp
    ./Core/Client/ClientCLI.cpp
  )
  target_link_libraries(resource_tuner_cli SocketCommClient ClientAPIs)
  install(TARGETS resource_tuner_cli RUNTIME DESTINATION bin)
endif()

# Server Executable
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
if(BUILD_SIGNALS)
  add_executable(resource_tuner ./ServerMain.cpp ./Signals/SignalInit.cpp)
  target_include_directories(resource_tuner PRIVATE ${YAMLCPP_INCLUDE_DIRS})
  target_include_directories(resource_tuner PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
  target_link_libraries(resource_tuner PUBLIC SysSignals)
  target_link_libraries(resource_tuner PUBLIC ExtAPIs)
else()
  add_executable(resource_tuner ./ServerMain.cpp)
  target_include_directories(resource_tuner PRIVATE ${YAMLCPP_INCLUDE_DIRS})
  target_include_directories(resource_tuner PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
  target_link_libraries(resource_tuner PUBLIC ExtAPIs)
endif()

target_link_libraries(resource_tuner PUBLIC Server Auxiliary SysRequestReceiver)

# System Test Executable
if(BUILD_TESTS)
  add_executable(sys_tests_ex ./Tests/System/SystemWideTests.cpp)
  target_link_libraries(sys_tests_ex PUBLIC Auxiliary ClientAPIs)
endif()

#install libs
install(TARGETS resource_tuner RUNTIME DESTINATION bin)
install(TARGETS Auxiliary SocketCommClient SocketCommServer
        ClientAPIs SysConfigProps Server ExtAPIs
        SysRequestReceiver LIBRARY DESTINATION lib)
if(BUILD_SIGNALS)
  install(TARGETS SysSignals DESTINATION lib)
endif()

#install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Client/APIs/Include/ DESTINATION include/ResourceTuner)

#install config files
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Configs/
    DESTINATION /etc/ResourceTuner/Common/
    FILES_MATCHING
    PATTERN "*.yaml"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/Signals/Configs/
    DESTINATION /etc/ResourceTuner/Common/
    FILES_MATCHING
    PATTERN "*.yaml"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
