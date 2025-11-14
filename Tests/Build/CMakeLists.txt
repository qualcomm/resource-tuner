if(BUILD_TESTS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

    file(GLOB SOURCES "../Utils/*.cpp")
    add_library(TestSetupUtils ${SOURCES})
    set_target_properties(TestSetupUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(TestSetupUtils Auxiliary)
    target_include_directories(TestSetupUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/Include)

    add_executable(resource_tuner_tests ../Integration/IntegrationTests.cpp)
    target_link_libraries(resource_tuner_tests PUBLIC Auxiliary ClientAPIs TestSetupUtils ${LIBYAML_LIBRARIES})
    target_include_directories(resource_tuner_tests PRIVATE ${LIBYAML_INCLUDE_DIRS})
    install(TARGETS resource_tuner_tests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(StateDetectionTests ../Integration/StateDetectionTests.cpp)
    target_link_libraries(StateDetectionTests PUBLIC Auxiliary ClientAPIs TestSetupUtils)
    install(TARGETS StateDetectionTests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(MemoryPoolTests ../Component/Comp/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests Auxiliary TestSetupUtils)
    install(TARGETS MemoryPoolTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(SafeOpsTests ../Component/Comp/SafeOpsTests.cpp)
    target_link_libraries(SafeOpsTests Auxiliary TestSetupUtils)
    install(TARGETS SafeOpsTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(ThreadPoolTests ../Component/Comp/ThreadPoolTests.cpp)
    target_link_libraries(ThreadPoolTests Auxiliary TestSetupUtils)
    install(TARGETS ThreadPoolTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(TimerTests ../Component/Comp/TimerTests.cpp)
    target_link_libraries(TimerTests Auxiliary TestSetupUtils)
    install(TARGETS TimerTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(MiscTests ../Component/Comp/MiscTests.cpp)
    target_link_libraries(MiscTests Auxiliary TestSetupUtils)
    install(TARGETS MiscTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(ClientDataManagerTests ../Component/Comp/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests Auxiliary Server TestSetupUtils)
    install(TARGETS ClientDataManagerTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(RequestMapTests ../Component/Comp/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS RequestMapTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(RateLimiterTests ../Component/Comp/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS RateLimiterTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(ResourceProcessorTests ../Component/Parsers/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS ResourceProcessorTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(ResourceParsingTests ../Component/Parsers/ResourceParsingTests.cpp)
    target_link_libraries(ResourceParsingTests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS ResourceParsingTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(TargetConfigProcessorTests ../Component/Parsers/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS TargetConfigProcessorTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(SysConfigAPITests ../Component/Parsers/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests Auxiliary Server ExtAPIs TestSetupUtils)
    install(TARGETS SysConfigAPITests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(ExtensionIntfTests ../Component/Comp/ExtensionIntfTests.cpp)
    target_link_libraries(ExtensionIntfTests Auxiliary ExtAPIs Server TestSetupUtils)
    install(TARGETS ExtensionIntfTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(RequestQueueTests ../Component/Comp/RequestQueueTests.cpp)
    target_link_libraries(RequestQueueTests Auxiliary ExtAPIs Server TestSetupUtils)
    install(TARGETS RequestQueueTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(InitConfigParsingTests ../Component/Parsers/InitConfigParsingTests.cpp)
    target_link_libraries(InitConfigParsingTests Auxiliary ExtAPIs Server TestSetupUtils)
    install(TARGETS InitConfigParsingTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(CocoTableTests ../Component/Comp/CocoTableTests.cpp)
    target_link_libraries(CocoTableTests Auxiliary ExtAPIs Server TestSetupUtils)
    install(TARGETS CocoTableTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(DeviceInfoTests ../Component/Device/DeviceInfoTests.cpp)
    target_link_libraries(DeviceInfoTests Auxiliary ExtAPIs Server TestSetupUtils ${LIBYAML_LIBRARIES})
    target_include_directories(DeviceInfoTests PRIVATE ${LIBYAML_INCLUDE_DIRS})
    install(TARGETS DeviceInfoTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(BUILD_SIGNALS)
        add_executable(SignalConfigProcessorTests ../Component/Parsers/SignalConfigProcessorTests.cpp)
        target_link_libraries(SignalConfigProcessorTests Auxiliary ExtAPIs SysSignals TestSetupUtils)
        install(TARGETS SignalConfigProcessorTests DESTINATION ${CMAKE_INSTALL_BINDIR})

        add_executable(SignalParsingTests ../Component/Parsers/SignalParsingTests.cpp)
        target_link_libraries(SignalParsingTests Auxiliary ExtAPIs SysSignals TestSetupUtils)
        install(TARGETS SignalParsingTests DESTINATION ${CMAKE_INSTALL_BINDIR})

        add_executable(ExtFeaturesParsingTests ../Component/Parsers/ExtFeaturesParsingTests.cpp)
        target_link_libraries(ExtFeaturesParsingTests Auxiliary ExtAPIs SysSignals TestSetupUtils)
        install(TARGETS ExtFeaturesParsingTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    endif()
endif()
