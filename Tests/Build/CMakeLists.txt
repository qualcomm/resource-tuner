if(BUILD_TESTS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

    file(GLOB SOURCES "../Utils/*.cpp")
    add_library(TestSetupUtils ${SOURCES})
    set_target_properties(TestSetupUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(TestSetupUtils Auxiliary)
    target_include_directories(TestSetupUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/Include)
    install(TARGETS TestSetupUtils DESTINATION ${CMAKE_INSTALL_LIBDIR})

    add_executable(RestuneIntegrationTests ../Integration/IntegrationTests.cpp)
    target_link_libraries(RestuneIntegrationTests PUBLIC Auxiliary ClientAPIs TestSetupUtils ${LIBYAML_LIBRARIES})
    target_include_directories(RestuneIntegrationTests PRIVATE ${LIBYAML_INCLUDE_DIRS})
    install(TARGETS RestuneIntegrationTests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(
        RestuneComponentTests
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/MemoryPoolTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ClientDataManagerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ParserTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ExtensionIntfTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/MiscTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RateLimiterTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RequestQueueTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ThreadPoolTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RequestMapTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/CocoTableTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/SafeOpsTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/TimerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/DeviceInfoTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/Trigger.cpp)
    target_link_libraries(RestuneComponentTests Auxiliary ExtAPIs Server SysSignals TestSetupUtils)
    install(TARGETS RestuneComponentTests DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
