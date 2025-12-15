cmake_minimum_required(VERSION 3.6)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17 LANGUAGES CXX)
set(CMAKE_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-Wall -Wextra)

project("URM")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Custom Build Options
option(BUILD_CLASSIFIER "Classifier" OFF)

# Install URM Config files
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/configs/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/resource-tuner/common/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install MODULA / AUX UTILS Lib
file(GLOB SOURCES "modula/Components/*.cpp"
                  "modula/Common/*.cpp"
                  "modula/CoreModules/*.cpp")

add_library(UrmAuxUtils ${SOURCES})
set_target_properties(UrmAuxUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(UrmAuxUtils PRIVATE ${LIBYAML_LIBRARIES})
target_include_directories(UrmAuxUtils PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(UrmAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/Components/Include)
target_include_directories(UrmAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/Common/Include)
target_include_directories(UrmAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/CoreModules/Include)
install(TARGETS UrmAuxUtils LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install URM Client Lib
file(GLOB SOURCES "client/APIs/*.cpp"
                  "client/Comm/Socket/*.cpp")
add_library(UrmClient ${SOURCES})
set_target_properties(UrmClient PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(UrmClient PRIVATE UrmAuxUtils)
target_include_directories(UrmClient PUBLIC client/APIs/Include)
target_include_directories(UrmClient PUBLIC client/Comm/Socket/Include)
target_include_directories(UrmClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modula/Common/Include)
install(TARGETS UrmClient LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# URM CLI Client
add_executable(urmClientCmd
  ${CMAKE_CURRENT_SOURCE_DIR}/client/APIs/ResourceTunerAPIs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/client/ClientCLI.cpp
)
target_link_libraries(urmClientCmd PRIVATE UrmAuxUtils)
target_link_libraries(urmClientCmd PRIVATE UrmClient)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/client/APIs/Include)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/Components/Include)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/Common/Include)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/modula/CoreModules/Include)
target_include_directories(urmClientCmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/client/Comm/Socket/Include)
install(TARGETS urmClientCmd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# resource-tuner is a mandatory build
add_subdirectory(${CMAKE_SOURCE_DIR}/resource-tuner)

# build classifier conditionally
if(BUILD_CLASSIFIER)
  add_subdirectory(${CMAKE_SOURCE_DIR}/contextual-classifier)
endif()

# Install Rules for Headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Client/APIs/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Extensions/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Modula/Common/Include/Common.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)
