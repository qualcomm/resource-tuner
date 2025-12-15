# Mandatory Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

# Optional Modules
option(BUILD_TESTS "Testing" OFF)
option(BUILD_STATE_DETECTOR "Display Detector" OFF)

# Mandatory Builds
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Signals)

if(BUILD_TESTS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests)
endif()

# Create the main resource-tuner executable
set(SERVER_EXECUTABLE "")
list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/ServerMain.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/Signals/SignalInit.cpp)

set(SERVER_LIBS "")
list(APPEND SERVER_LIBS
  UrmAuxUtils
  UrmReceiver
  RestuneServer
  RestuneExtAPIs
  RestuneSignals
)

if(BUILD_STATE_DETECTOR)
  pkg_check_modules(SYSTEMD REQUIRED libsystemd)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/StateDetector.cpp)
  list(APPEND SERVER_LIBS ${SYSTEMD_LIBRARIES})
endif()

add_executable(resource_tuner ${SERVER_EXECUTABLE} ${CMAKE_SOURCE_DIR})
target_link_libraries(resource_tuner PUBLIC ${SERVER_LIBS})

# Install bin
install(TARGETS resource_tuner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
