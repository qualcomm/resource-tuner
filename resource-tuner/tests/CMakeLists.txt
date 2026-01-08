if(BUILD_TESTS)
    # Install Test Configs
    file(GLOB testConfigs "${CMAKE_CURRENT_SOURCE_DIR}/Configs/*.yaml")
    install(
        FILES ${testConfigs}
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/urm/tests/configs/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )

    # Install Custom Resource Nodes
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Configs/ResourceSysFsNodes/
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/urm/tests/nodes/
        FILES_MATCHING
        PATTERN "*.txt"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )

    add_library(RestuneTestUtils
                ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Baseline.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/Utils/TestAggregator.cpp)
    set_target_properties(RestuneTestUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(RestuneTestUtils UrmAuxUtils)
    target_include_directories(RestuneTestUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Include)
    install(TARGETS RestuneTestUtils DESTINATION ${CMAKE_INSTALL_LIBDIR})

    add_library(RestunePlugin ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Setup.cpp)
    set_target_properties(RestunePlugin PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(RestunePlugin UrmExtAPIs)
    install(TARGETS RestunePlugin DESTINATION ${CMAKE_INSTALL_LIBDIR})

    add_executable(RestuneIntegrationTests Integration/IntegrationTests.cpp)
    target_link_libraries(RestuneIntegrationTests PUBLIC UrmAuxUtils
                                                         UrmClient
                                                         RestuneTestUtils
                                                         ${LIBYAML_LIBRARIES})
    target_include_directories(RestuneIntegrationTests PRIVATE ${LIBYAML_INCLUDE_DIRS})
    install(TARGETS RestuneIntegrationTests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(
        RestuneComponentTests
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/Trigger.cpp)

    target_link_libraries(RestuneComponentTests PUBLIC UrmAuxUtils
                                                       UrmExtAPIs
                                                       RestuneCore
                                                       RestuneTestUtils)

    install(TARGETS RestuneComponentTests DESTINATION ${CMAKE_INSTALL_BINDIR})

    
    # --- Mini.hpp header-only unit/module/usecase tests ---
    # Source list for mini.hpp-based tests (add more .cpps here as you create them)
    set(MINI_TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/tests_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/ThreadPoolTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/TimerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/SafeOpsTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/RequestQueueTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/RateLimiterTests.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/Component/ParserTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/MiscTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/MemoryPoolTests.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/Component/ExtensionIntfTests.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/Component/DeviceInfoTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Component/RequestMapTests.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/Component/CocoTableTests.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/Component/ClientDataManagerTests.cpp

        # Example: ${CMAKE_CURRENT_SOURCE_DIR}/Module/cache_tests.cpp
        # Example: ${CMAKE_CURRENT_SOURCE_DIR}/Usecase/login_flow_tests.cpp
    )

    # Create a single test runner binary that uses mini.hpp's built-in main()
    add_executable(RestuneMiniTests ${MINI_TEST_SOURCES})

    # Make the header visible to the test sources
    target_include_directories(RestuneMiniTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/framework
    )

    # Link any production libraries your tests depend on (adjust as needed)
    # Reuse the same libs your component tests already link to:
    target_link_libraries(RestuneMiniTests PUBLIC
        UrmAuxUtils
        UrmExtAPIs
        RestuneCore
        RestuneTestUtils
        # If your tests need yaml, etc., add here:
        # ${LIBYAML_LIBRARIES}
    )

    # (Optional) define to default TAP output off; you can still enable via CLI
    target_compile_definitions(RestuneMiniTests PRIVATE
        MTEST_DEFAULT_OUTPUT_TAP=0
        # Enable exception short-circuiting if you want:
        # MTEST_ENABLE_EXCEPTIONS=1
    )

    # Install the test runner like your other test executables
    install(TARGETS RestuneMiniTests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Register CTest entries so you can run `ctest`:
    # A generic run (no filters)
    add_test(NAME RestuneMiniTests_All
             COMMAND RestuneMiniTests)

    # Tag-based slices (use the tags you put in MT_TEST / MT_TEST_F)
    add_test(NAME RestuneMiniTests_Unit
             COMMAND RestuneMiniTests --tag=component)

endif()
