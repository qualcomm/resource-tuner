if(BUILD_TESTS)

    add_executable(MemoryPoolTests ./Auxiliary/MemoryPoolTests.cpp)
    target_link_libraries(MemoryPoolTests GTest::gtest_main Auxiliary)

    add_executable(SafeOpsTest ./Auxiliary/SafeOpsTest.cpp)
    target_link_libraries(SafeOpsTest GTest::gtest_main Auxiliary)

    add_executable(ThreadPoolTest ./Auxiliary/ThreadPoolTest.cpp)
    target_link_libraries(ThreadPoolTest GTest::gtest_main Auxiliary)

    add_executable(TimerTest Auxiliary/TimerTest.cpp)
    target_link_libraries(TimerTest GTest::gtest_main Auxiliary)

    add_executable(MiscTests Auxiliary/MiscTests.cpp)
    target_link_libraries(MiscTests GTest::gtest_main Auxiliary)

    add_executable(JsonParserTest Auxiliary/JsonParserTest.cpp)
    target_link_libraries(JsonParserTest GTest::gtest_main Auxiliary)

    add_executable(RequestQueueTest Provisioner/RequestQueueTest.cpp)
    target_link_libraries(RequestQueueTest GTest::gtest_main Server)

    add_executable(CocoTableTest Provisioner/CocoTableTest.cpp)
    target_link_libraries(CocoTableTest GTest::gtest_main Server ExtAPIs)

    add_executable(ClientDataManagerTests Provisioner/ClientDataManagerTests.cpp)
    target_link_libraries(ClientDataManagerTests GTest::gtest_main Auxiliary Server)

    add_executable(RequestMapTests Provisioner/RequestMapTests.cpp)
    target_link_libraries(RequestMapTests GTest::gtest_main  Auxiliary Server)

    add_executable(RateLimiterTests Provisioner/RateLimiterTests.cpp)
    target_link_libraries(RateLimiterTests GTest::gtest_main  Auxiliary Server)

    add_executable(ResourceProcessorTests Provisioner/ResourceProcessorTests.cpp)
    target_link_libraries(ResourceProcessorTests GTest::gtest_main Auxiliary Server)

    add_executable(TargetConfigProcessorTests Provisioner/TargetConfigProcessorTests.cpp)
    target_link_libraries(TargetConfigProcessorTests GTest::gtest_main Auxiliary Server)

    add_executable(SysConfigAPITests SysConfigs/SysConfigAPITests.cpp)
    target_link_libraries(SysConfigAPITests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    add_executable(SysConfigProcessorTests SysConfigs/SysConfigProcessorTests.cpp)
    target_link_libraries(SysConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysConfigProps)

    if(BUILD_SIGNALS)
        add_executable(SysSignalConfigProcessorTests SysSignals/SignalConfigProcessorTests.cpp)
        target_link_libraries(SysSignalConfigProcessorTests GTest::gtest_main Auxiliary ExtAPIs SysSignals)
    endif()

    include(GoogleTest)
    gtest_discover_tests(RequestQueueTest)
    gtest_discover_tests(CocoTableTest)
    gtest_discover_tests(ClientDataManagerTests)
    gtest_discover_tests(RequestMapTests)
    gtest_discover_tests(RateLimiterTests)
    gtest_discover_tests(MemoryPoolTests)
    gtest_discover_tests(SafeOpsTest)
    gtest_discover_tests(MiscTests)
    gtest_discover_tests(ThreadPoolTest)
    gtest_discover_tests(TimerTest)
    gtest_discover_tests(JsonParserTest)
    gtest_discover_tests(SysConfigAPITests)
    gtest_discover_tests(ResourceProcessorTests)
    gtest_discover_tests(TargetConfigProcessorTests)
    if(BUILD_SIGNALS)
        gtest_discover_tests(SysSignalConfigProcessorTests)
    endif()
endif()
