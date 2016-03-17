find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(${Boost_INCLUDE_DIRS})

add_library(test_common
        temporary_file.h temporary_file.cpp
        file_operations.h file_operations.cpp)

target_link_libraries(test_common
        ${Boost_LIBRARIES})