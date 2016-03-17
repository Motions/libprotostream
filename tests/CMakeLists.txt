option(BUILD_TESTS "Build tests" ON)

if (BUILD_TESTS)
    add_subdirectory(gtest)
    add_subdirectory(common)
    add_subdirectory(integrationtests)
    add_subdirectory(unittests)

    enable_testing()
    add_test(unittests unittests/unittests)
    add_test(integrationtests integrationtests/integrationtests)
endif ()