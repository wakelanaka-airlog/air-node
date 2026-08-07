# GoogleTest-based unit tests for the hardware-agnostic logic in node/.
# GoogleTest is only fetched once there is actually something to build
# against it, so an empty tests/ directory configures instantly and offline.

file(GLOB_RECURSE SENSOR_NODE_TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

if(SENSOR_NODE_TEST_SOURCES)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(sensor_node_tests ${SENSOR_NODE_TEST_SOURCES})
    target_include_directories(sensor_node_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(sensor_node_tests PRIVATE sensor_node GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(sensor_node_tests)
else()
    message(STATUS "sensor_node: no test sources yet in tests/ - skipping GoogleTest setup")
endif()
