# Mandatory Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

# Optional Modules
option(BUILD_TESTS "Testing" OFF)

# EXTENSIONS
file(GLOB SOURCES "extensions/Extensions.cpp")
add_library(RestuneExtAPIs ${SOURCES})
set_target_properties(RestuneExtAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneExtAPIs PRIVATE UrmAuxUtils)
target_include_directories(RestuneExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extensions/Include)
install(TARGETS RestuneExtAPIs LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extensions/Include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ResourceTuner)

# Restune
file(GLOB SOURCES "core/*.cpp"
                  "core/Server/*.cpp"
                  "signals/*.cpp"
                  "init/*.cpp")
add_library(RestuneCore ${SOURCES})
set_target_properties(RestuneCore PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneCore PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_link_libraries(RestuneCore PRIVATE UrmAuxUtils
                                          RestuneExtAPIs
                                          ${LIBYAML_LIBRARIES})

target_include_directories(RestuneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/Include)
target_include_directories(RestuneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/Server/Include)
target_include_directories(RestuneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/signals/Include)
target_include_directories(RestuneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/init/Include)
install(TARGETS RestuneCore LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(BUILD_TESTS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()
