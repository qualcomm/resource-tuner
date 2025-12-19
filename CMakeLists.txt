cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 17 LANGUAGES CXX)
set(CMAKE_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-Wall -Wextra)

project("URM")

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
# Custom Build Options
option(BUILD_CLASSIFIER "Classifier" OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/configs)
add_subdirectory(${CMAKE_SOURCE_DIR}/modula)
add_subdirectory(${CMAKE_SOURCE_DIR}/client)

# resource-tuner is a mandatory build
add_subdirectory(${CMAKE_SOURCE_DIR}/resource-tuner)

# Create and install the urm executable
set(SERVER_EXECUTABLE "")
list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/urm.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/resource-tuner/init/RestuneInit.cpp)

# build classifier conditionally
if(BUILD_CLASSIFIER)
#  pkg_check_modules(FASTTEXT REQUIRED fasttext)
    find_package(fasttext CONFIG)
    if(fastText_FOUND)
        message(STATUS "Found fastText: ${fastText_INCLUDE_DIRS}")
        include_directories(${fastText_INCLUDE_DIRS})
    else()
        message(WARNING "fastText not found. Please install it or set fastText_DIR.")
    endif()

  add_subdirectory(${CMAKE_SOURCE_DIR}/contextual-classifier)
  list(APPEND SERVER_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/contextual-classifier/ContextualClassifier.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/contextual-classifier/NetLinkComm.cpp)
endif()

set(SERVER_LIBS "")
list(APPEND SERVER_LIBS
  UrmAuxUtils
  RestuneCore
  RestuneExtAPIs
)

if(BUILD_CLASSIFIER)
  list(APPEND SERVER_LIBS parser ml_inference_lib fasttext dl)
endif()

add_executable(urm_exec ${SERVER_EXECUTABLE} ${CMAKE_SOURCE_DIR})
target_link_libraries(urm_exec PUBLIC ${SERVER_LIBS})

# Install bin
install(TARGETS urm_exec RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
