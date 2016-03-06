cmake_minimum_required(VERSION 2.8)

project(libprotostream)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
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

