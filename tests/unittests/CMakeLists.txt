add_executable(unittests
        test_file_backend.cpp
        test_file_header.cpp
        test_header_magic_value.cpp
        test_header_with_offset.cpp
        test_posix_file_handler.cpp
        test_reduced_keyframe_header.cpp
        test_utils.cpp
        test_cache_base.cpp mock_backend.h mock_backend.h mock_cache.h)

target_link_libraries(unittests
        protostream
        test_common
        gtest
        gmock
        gmock_main)
