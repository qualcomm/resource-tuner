cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 17 LANGUAGES CXX)
set(CMAKE_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-Wall -Wextra)

project("URM")

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Custom Build Options
option(BUILD_CLASSIFIER "Classifier" OFF)
option(BUILD_STATE_DETECTOR "Display Detector" OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/configs)
add_subdirectory(${CMAKE_SOURCE_DIR}/modula)
add_subdirectory(${CMAKE_SOURCE_DIR}/client)

# resource-tuner is a mandatory build
add_subdirectory(${CMAKE_SOURCE_DIR}/resource-tuner)

# build classifier conditionally
if(BUILD_CLASSIFIER)
  add_subdirectory(${CMAKE_SOURCE_DIR}/contextual-classifier)
endif()

# Create and install the urm executable
set(SERVER_EXECUTABLE "")
list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/urm.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/resource-tuner/init/RestuneInit.cpp)

set(SERVER_LIBS "")
list(APPEND SERVER_LIBS
  UrmAuxUtils
  RestuneCore
  RestuneExtAPIs
)

if(BUILD_STATE_DETECTOR)
  pkg_check_modules(SYSTEMD REQUIRED libsystemd)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/resource-tuner/StateDetector.cpp)
  list(APPEND SERVER_LIBS ${SYSTEMD_LIBRARIES})
endif()

if(BUILD_CLASSIFIER)
  # list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/contextual-classifier/classifier.cpp)
  # list(APPEND SERVER_LIBS classifierLib)
endif()

add_executable(urm_exec ${SERVER_EXECUTABLE} ${CMAKE_SOURCE_DIR})
target_link_libraries(urm_exec PUBLIC ${SERVER_LIBS})

# Install bin
install(TARGETS urm_exec RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
