cmake_minimum_required(VERSION 3.1)

project(libprotostream)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O2")

add_subdirectory(include)

add_executable(testmain test/testmain.cpp)
target_link_libraries(testmain protostream)

add_library(cprotostream SHARED src/cprotostream.cpp)
target_link_libraries(cprotostream protostream)

add_library(cprotostream_static STATIC src/cprotostream.cpp)
target_link_libraries(cprotostream_static protostream)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    set(TEST_REPORTS_DIR "${CMAKE_CURRENT_BINARY_DIR}/test_reports")
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/tests")
    file(MAKE_DIRECTORY "${TEST_REPORTS_DIR}")

    add_test(NAME unittests
            COMMAND
                "${TEST_DIR}/unittests/unittests"
                "--gtest_output=xml:${TEST_REPORTS_DIR}/unittests.xml"
            WORKING_DIRECTORY "${TEST_DIR}/unittests")

    add_test(NAME integrationtests
            COMMAND
                "${TEST_DIR}/integrationtests/integrationtests"
                "--gtest_output=xml:${TEST_REPORTS_DIR}/integrationtests.xml"
            WORKING_DIRECTORY "${TEST_DIR}/integrationtests")
endif ()