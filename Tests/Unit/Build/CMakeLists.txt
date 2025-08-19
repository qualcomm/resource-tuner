if(BUILD_TESTS)
    add_executable(YamlParserTests ../Core/YamlParserTests.cpp)
    target_link_libraries(YamlParserTests GTest::gtest_main Auxiliary)

    add_executable(MemoryPoolTests ../Core/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests GTest::gtest_main Auxiliary)

    add_executable(SafeOpsTest ../Core/SafeOpsTest.cpp)
    target_link_libraries(SafeOpsTest GTest::gtest_main Auxiliary)

    add_executable(ThreadPoolTests ../Core/ThreadPoolTests.cpp)
    target_link_libraries(ThreadPoolTests GTest::gtest_main Auxiliary)

    add_executable(TimerTest ../Core/TimerTest.cpp)
    target_link_libraries(TimerTest GTest::gtest_main Auxiliary)

    add_executable(MiscTests ../Core/MiscTests.cpp)
    target_link_libraries(MiscTests GTest::gtest_main Auxiliary)

    add_executable(RequestQueueTest ../Core/RequestQueueTest.cpp)
    target_link_libraries(RequestQueueTest GTest::gtest_main Server)

    add_executable(CocoTableTest ../Core/CocoTableTest.cpp)
    target_link_libraries(CocoTableTest GTest::gtest_main Server ExtAPIs)

    add_executable(ClientDataManagerTests ../Core/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests GTest::gtest_main Auxiliary Server)

    add_executable(RequestMapTests ../Core/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests GTest::gtest_main  Auxiliary Server ExtAPIs)

    add_executable(RateLimiterTests ../Core/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests GTest::gtest_main  Auxiliary Server ExtAPIs)

    add_executable(ResourceProcessorTests ../Core/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests GTest::gtest_main Auxiliary Server ExtAPIs)

    add_executable(TargetConfigProcessorTests ../Core/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests GTest::gtest_main Auxiliary Server ExtAPIs)

    add_executable(SysConfigAPITests ../Core/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    add_executable(SysConfigProcessorTests ../Core/SysConfigProcessorTests.cpp)
    target_link_libraries(SysConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    if(BUILD_SIGNALS)
        add_executable(SysSignalConfigProcessorTests ../Signals/SignalConfigProcessorTests.cpp)
        target_link_libraries(SysSignalConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysSignals)
    endif()

    if(NOT CMAKE_CROSSCOMPILING)
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
endif()
