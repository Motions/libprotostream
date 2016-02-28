cmake_minimum_required(VERSION 2.8)

project(libprotostream)

set(CMAKE_CXX_FLAGS "-Wall -std=c++14 -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

add_subdirectory(src)

include_directories("${PROJECT_SOURCE_DIR}/src")
add_executable(testmain test/testmain.cpp)
#add_executable(testmain src/test.cc)
target_link_libraries(testmain protostream_static)

