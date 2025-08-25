if(BUILD_TESTS)
    add_executable(YamlParserTests ../Core/YamlParserTests.cpp)
    target_link_libraries(YamlParserTests Auxiliary)

    add_executable(MemoryPoolTests ../Core/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests Auxiliary)

    add_executable(SafeOpsTest ../Core/SafeOpsTest.cpp)
    target_link_libraries(SafeOpsTest Auxiliary)

    add_executable(ThreadPoolTests ../Core/ThreadPoolTests.cpp)
    target_link_libraries(ThreadPoolTests Auxiliary)

    add_executable(TimerTest ../Core/TimerTest.cpp)
    target_link_libraries(TimerTest Auxiliary)

    add_executable(MiscTests ../Core/MiscTests.cpp)
    target_link_libraries(MiscTests Auxiliary)

    add_executable(ClientDataManagerTests ../Core/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests Auxiliary Server)

    add_executable(RequestMapTests ../Core/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests Auxiliary Server ExtAPIs)

    add_executable(RateLimiterTests ../Core/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests Auxiliary Server ExtAPIs)

    add_executable(ResourceProcessorTests ../Core/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests Auxiliary Server ExtAPIs)

    add_executable(ResourceParsingTests ../Core/ResourceParsingTests.cpp)
    target_link_libraries(ResourceParsingTests Auxiliary Server ExtAPIs)

    add_executable(TargetConfigProcessorTests ../Core/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests Auxiliary Server ExtAPIs)

    add_executable(SysConfigAPITests ../Core/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests Auxiliary ExtAPIs SysConfigProps)

    add_executable(ExtensionIntfTests ../Core/ExtensionIntfTests.cpp)
    target_link_libraries(ExtensionIntfTests Auxiliary ExtAPIs Server)

    if(BUILD_SIGNALS)
        add_executable(SysSignalConfigProcessorTests ../Signals/SignalConfigProcessorTests.cpp)
        target_link_libraries(SysSignalConfigProcessorTests Auxiliary ExtAPIs SysSignals)

        add_executable(SignalParsingTests ../Signals/SignalParsingTests.cpp)
        target_link_libraries(SignalParsingTests Auxiliary ExtAPIs SysSignals)
    endif()
endif()
