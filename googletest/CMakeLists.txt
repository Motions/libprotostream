find_package(GTest)
find_package(Boost COMPONENTS system filesystem)
if(Boost_FOUND AND GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(googletest
            test_posix_file_handler.cpp
            test_utils.cpp
            temporary_file.h
            test_file_backend.cpp
            test_header_magic_value.cpp
            test_header_with_offset.cpp test_file_header.cpp test_reduced_keyframe_header.cpp)

    target_link_libraries(googletest
            protostream
            ${GTEST_BOTH_LIBRARIES}
            ${Boost_LIBRARIES})
endif()