find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(googletest
        test_posix_file_handler.cpp
        test_utils.cpp
        temporary_file.h
        test_file_backend.cpp
        test_header_magic_value.cpp
        test_header_with_offset.cpp)

target_link_libraries(googletest
        protostream
        ${GTEST_BOTH_LIBRARIES}
        ${Boost_LIBRARIES})
