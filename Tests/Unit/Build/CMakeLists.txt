if(BUILD_TESTS)

    add_executable(YamlParserTests ../Core/YamlParserTests.cpp)
    target_link_libraries(YamlParserTests Auxiliary)
    install(TARGETS YamlParserTests DESTINATION bin)

    add_executable(MemoryPoolTests ../Core/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests Auxiliary)
    install(TARGETS MemoryPoolTests DESTINATION bin)

    add_executable(SafeOpsTests ../Core/SafeOpsTests.cpp)
    target_link_libraries(SafeOpsTests Auxiliary)
    install(TARGETS SafeOpsTests DESTINATION bin)

    add_executable(ThreadPoolTests ../Core/ThreadPoolTests.cpp)
    target_link_libraries(ThreadPoolTests Auxiliary)
    install(TARGETS ThreadPoolTests DESTINATION bin)

    add_executable(TimerTests ../Core/TimerTests.cpp)
    target_link_libraries(TimerTests Auxiliary)
    install(TARGETS TimerTests DESTINATION bin)

    add_executable(MiscTests ../Core/MiscTests.cpp)
    target_link_libraries(MiscTests Auxiliary)
    install(TARGETS MiscTests DESTINATION bin)

    add_executable(ClientDataManagerTests ../Core/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests Auxiliary Server)
    install(TARGETS ClientDataManagerTests DESTINATION bin)

    add_executable(RequestMapTests ../Core/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests Auxiliary Server ExtAPIs)
    install(TARGETS RequestMapTests DESTINATION bin)

    add_executable(RateLimiterTests ../Core/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests Auxiliary Server ExtAPIs)
    install(TARGETS RateLimiterTests DESTINATION bin)

    add_executable(ResourceProcessorTests ../Core/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests Auxiliary Server ExtAPIs)
    install(TARGETS ResourceProcessorTests DESTINATION bin)

    add_executable(ResourceParsingTests ../Core/ResourceParsingTests.cpp)
    target_link_libraries(ResourceParsingTests Auxiliary Server ExtAPIs)
    install(TARGETS ResourceParsingTests DESTINATION bin)

    add_executable(TargetConfigProcessorTests ../Core/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests Auxiliary Server ExtAPIs)
    install(TARGETS TargetConfigProcessorTests DESTINATION bin)

    add_executable(SysConfigAPITests ../Core/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests Auxiliary Server ExtAPIs)
    install(TARGETS SysConfigAPITests DESTINATION bin)

    add_executable(ExtensionIntfTests ../Core/ExtensionIntfTests.cpp)
    target_link_libraries(ExtensionIntfTests Auxiliary ExtAPIs Server)
    install(TARGETS ExtensionIntfTests DESTINATION bin)

    add_executable(InitConfigParsingTests ../Core/InitConfigParsingTests.cpp)
    target_link_libraries(InitConfigParsingTests Auxiliary ExtAPIs Server)
    install(TARGETS InitConfigParsingTests DESTINATION bin)

    add_executable(CocoTableTests ../Core/CocoTableTests.cpp)
    target_link_libraries(CocoTableTests Auxiliary ExtAPIs Server)
    install(TARGETS CocoTableTests DESTINATION bin)

    if(BUILD_SIGNALS)
        add_executable(SysSignalConfigProcessorTests ../Signals/SignalConfigProcessorTests.cpp)
        target_link_libraries(SysSignalConfigProcessorTests Auxiliary ExtAPIs SysSignals)
        install(TARGETS SysSignalConfigProcessorTests DESTINATION bin)

        add_executable(SignalParsingTests ../Signals/SignalParsingTests.cpp)
        target_link_libraries(SignalParsingTests Auxiliary ExtAPIs SysSignals)
        install(TARGETS SignalParsingTests DESTINATION bin)
    endif()
endif()
