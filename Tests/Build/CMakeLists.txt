if(BUILD_TESTS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

    add_library(RestuneTestUtils
                ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/Baseline.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/TestAggregator.cpp)
    set_target_properties(RestuneTestUtils PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(RestuneTestUtils RestuneAuxUtils)
    target_include_directories(RestuneTestUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/Include)
    install(TARGETS RestuneTestUtils DESTINATION ${CMAKE_INSTALL_LIBDIR})

    add_library(RestunePlugin ${CMAKE_CURRENT_SOURCE_DIR}/../Utils/Setup.cpp)
    set_target_properties(RestunePlugin PROPERTIES VERSION 1.0.0 SOVERSION 1)
    target_link_libraries(RestunePlugin RestuneExts)
    install(TARGETS RestunePlugin DESTINATION ${CMAKE_INSTALL_LIBDIR})

    add_executable(RestuneIntegrationTests ../Integration/IntegrationTests.cpp)
    target_link_libraries(RestuneIntegrationTests PUBLIC RestuneAuxUtils RestuneClient RestuneTestUtils ${LIBYAML_LIBRARIES})
    target_include_directories(RestuneIntegrationTests PRIVATE ${LIBYAML_INCLUDE_DIRS})
    install(TARGETS RestuneIntegrationTests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_executable(
        RestuneComponentTests
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/MemoryPoolTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ClientDataManagerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ParserTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ExtensionIntfTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/MiscTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RequestQueueTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/ThreadPoolTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/SafeOpsTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RateLimiterTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/TimerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/DeviceInfoTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/CocoTableTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/RequestMapTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../Component/Trigger.cpp)
    target_link_libraries(RestuneComponentTests RestuneAuxUtils RestuneExts RestuneServer RestuneSignals RestuneTestUtils)
    install(TARGETS RestuneComponentTests DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
