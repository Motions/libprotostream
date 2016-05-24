add_executable(integrationtests
        simple_tests.h
        simple_tests.cpp
        streams.h
        type_list.h
        type_list.cpp

        test_read_simple.cpp
        test_write_simple.cpp
        test_read_error.cpp
        test_write_error.cpp)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(integrationtests
        protostream
        test_common
        gtest
        gtest_main)
