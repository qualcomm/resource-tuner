# CLIENT
file(GLOB SOURCES "../Client/APIs/*.cpp" "../Client/Comm/Socket/*.cpp")
add_library(RestuneClient ${SOURCES})
set_target_properties(RestuneClient PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneClient RestuneAuxUtils)
target_include_directories(RestuneClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/APIs/Include)
target_include_directories(RestuneClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Client/Comm/Socket/Include)

# EXTENSIONS
file(GLOB SOURCES "../Extensions/*.cpp")
add_library(RestuneExtAPIs ${SOURCES})
set_target_properties(RestuneExtAPIs PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneExtAPIs RestuneAuxUtils)
target_include_directories(RestuneExtAPIs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Extensions/Include)

# FRAMEWORK
file(GLOB SOURCES "../Framework/*.cpp")
add_library(RestuneServer ${SOURCES})
set_target_properties(RestuneServer PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneServer PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_include_directories(RestuneServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework/Include)
target_link_libraries(RestuneServer PRIVATE RestuneAuxUtils RestuneSockEnd RestuneExtAPIs ${LIBYAML_LIBRARIES})

# MODULA
file(GLOB SOURCES "../Modula/Components/*.cpp" "../Modula/Common/*.cpp" "../Modula/CoreModules/*.cpp")
add_library(RestuneAuxUtils ${SOURCES})
set_target_properties(RestuneAuxUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneAuxUtils PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_link_libraries(RestuneAuxUtils PRIVATE ${LIBYAML_LIBRARIES})

target_include_directories(RestuneAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Components/Include)
target_include_directories(RestuneAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/Common/Include)
target_include_directories(RestuneAuxUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Modula/CoreModules/Include)

# SERVER / RECEIVER ENDPOINT
file(GLOB SOURCES "../Server/Receiver/*.cpp")
add_library(RestuneReceiver ${SOURCES})
set_target_properties(RestuneReceiver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneReceiver RestuneAuxUtils RestuneSockEnd)
target_include_directories(RestuneReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Receiver/Include)

file(GLOB SOURCES "../Server/Socket/*.cpp")
add_library(RestuneSockEnd ${SOURCES})
set_target_properties(RestuneSockEnd PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestuneSockEnd RestuneAuxUtils)
target_include_directories(RestuneSockEnd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Server/Socket/Include)
