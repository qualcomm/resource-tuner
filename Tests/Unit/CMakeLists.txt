if(BUILD_TESTS)
    add_executable(YamlParserTests Modula/YamlParserTests.cpp)
    target_link_libraries(YamlParserTests GTest::gtest_main Auxiliary)

    add_executable(MemoryPoolTests ./Modula/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests GTest::gtest_main Auxiliary)

    add_executable(SafeOpsTest ./Modula/SafeOpsTest.cpp)
    target_link_libraries(SafeOpsTest GTest::gtest_main Auxiliary)

    add_executable(ThreadPoolTests ./Modula/ThreadPoolTests.cpp)
    target_link_libraries(ThreadPoolTests GTest::gtest_main Auxiliary)

    add_executable(TimerTest Modula/TimerTest.cpp)
    target_link_libraries(TimerTest GTest::gtest_main Auxiliary)

    add_executable(MiscTests Modula/MiscTests.cpp)
    target_link_libraries(MiscTests GTest::gtest_main Auxiliary)

    add_executable(RequestQueueTest Framework/RequestQueueTest.cpp)
    target_link_libraries(RequestQueueTest GTest::gtest_main Server)

    add_executable(CocoTableTest Framework/CocoTableTest.cpp)
    target_link_libraries(CocoTableTest GTest::gtest_main Server ExtAPIs)

    add_executable(ClientDataManagerTests Framework/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests GTest::gtest_main Auxiliary Server)

    add_executable(RequestMapTests Framework/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests GTest::gtest_main  Auxiliary Server)

    add_executable(RateLimiterTests Framework/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests GTest::gtest_main  Auxiliary Server)

    add_executable(ResourceProcessorTests Framework/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests GTest::gtest_main Auxiliary Server ExtAPIs)

    add_executable(TargetConfigProcessorTests Framework/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests GTest::gtest_main Auxiliary Server ExtAPIs)

    add_executable(SysConfigAPITests SysConfigs/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    add_executable(SysConfigProcessorTests SysConfigs/SysConfigProcessorTests.cpp)
    target_link_libraries(SysConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    if(BUILD_SIGNALS)
        add_executable(SysSignalConfigProcessorTests SysSignals/SignalConfigProcessorTests.cpp)
        target_link_libraries(SysSignalConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysSignals)
    endif()

    include(GoogleTest)
    gtest_discover_tests(YamlParserTests)
    gtest_discover_tests(RequestQueueTest)
    gtest_discover_tests(CocoTableTest)
    gtest_discover_tests(ClientDataManagerTests)
    gtest_discover_tests(RequestMapTests)
    gtest_discover_tests(RateLimiterTests)
    gtest_discover_tests(MemoryPoolTests)
    gtest_discover_tests(SafeOpsTest)
    gtest_discover_tests(MiscTests)
    gtest_discover_tests(ThreadPoolTests)
    gtest_discover_tests(TimerTest)
    gtest_discover_tests(SysConfigAPITests)
    gtest_discover_tests(ResourceProcessorTests)
    gtest_discover_tests(TargetConfigProcessorTests)
    if(BUILD_SIGNALS)
        gtest_discover_tests(SysSignalConfigProcessorTests)
    endif()
endif()
