add_executable(integrationtests
        test_read_small.cpp
        common.h test_write_small.cpp)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(integrationtests
        protostream
        test_common
        gtest
        gtest_main)