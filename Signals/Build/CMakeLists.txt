file(GLOB SOURCES "../*.cpp")

add_library(RestuneSignals ${SOURCES})
set_target_properties(RestuneSignals PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(RestuneSignals PRIVATE ${CMAKE_SOURCE_DIR}/Extensions/Include)
target_include_directories(RestuneSignals PRIVATE ${LIBYAML_INCLUDE_DIRS})
target_link_libraries(RestuneSignals PUBLIC RestuneServer RestuneAuxUtils RestuneExtAPIs ${LIBYAML_LIBRARIES})
target_include_directories(RestuneSignals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Include)
