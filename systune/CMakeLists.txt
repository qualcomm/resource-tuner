cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)
# for gdb. can be removed in production.
set(CMAKE_BUILD_TYPE Debug)
# no need to run tests for external library jsoncpp.
set(JSONCPP_WITH_TESTS OFF)

project(Systune)

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
  jsoncpp
  GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(jsoncpp)

# Detect AArch64 cross-compilation
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    message(STATUS "Note:Cross-compiling for AArch64 — building libcpuinfo from source")
    add_compile_definitions(CROSS_COMPILE_BUILD="TRUE")

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/syslock_aarch64" CACHE PATH "Install path for AArch64 target" FORCE)
    endif()
endif()

# Enable Doxygen documentation build
# first we can indicate the documentation build as an option and set it to OFF by default
option(BUILD_DOC "Build documentation" OFF)
if(BUILD_DOC)
# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
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
add_subdirectory(Auxiliary)
add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(Extensions)
add_subdirectory(Provisioner)

# Optional builds
option(BUILD_SIGNALS "Build SysSignals" OFF)
if(BUILD_SIGNALS)
add_compile_definitions(SYS_SIGNALS_PLUGGED_IN=1)
add_subdirectory(Signals)
endif()

option(BUILD_TESTS "Testing" OFF)
if(BUILD_TESTS)
enable_testing()
add_compile_definitions(UNIT_TESTS_BUILD="TRUE")
add_subdirectory(Tests)
endif()

option(BUILD_CLI "Build CLI" OFF)
if(BUILD_CLI)
add_executable(systune_cli
  ./Client/APIs/SystuneAPIs.cpp
  ./Client/ClientCLI.cpp
)
target_link_libraries(systune_cli CommunicationClient ClientAPIs)
endif()

# Executables

# Server Executable
if(BUILD_SIGNALS)
  add_executable(systune ./ServerMain.cpp ./Signals/SignalServer.cpp)
  target_link_libraries(systune PUBLIC SysSignals)
else()
  add_executable(systune ./ServerMain.cpp)
endif()

target_link_libraries(systune PUBLIC Server Auxiliary SysRequestReceiver)

# System Test Executable
if(BUILD_TESTS)
add_executable(sys_tests_ex ./Tests/System/SystemWideTests.cpp)
target_link_libraries(sys_tests_ex PUBLIC Auxiliary ClientAPIs)
endif()

install(TARGETS systune RUNTIME DESTINATION bin)
install(TARGETS Auxiliary CommunicationClient SystuneCommunicationServer ClientAPIs SysConfigProps Server ExtAPIs LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Auxiliary/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Client/Provisioner/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Provisioner/Comm/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Provisioner/Server/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Extensions/Provisioner/include/ DESTINATION include)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Provisioner/SysConfigs DESTINATION .)
