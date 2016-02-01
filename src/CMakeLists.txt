cmake_minimum_required(VERSION 2.8)

#project(libprotostream)

set (SOURCES
    common.cpp
    stream_reader.cpp
    stream_writer.cpp
    mmap_reader.cpp
)


# static library for internal/test use
add_library(protostream_static STATIC ${SOURCES})
add_library(protostream SHARED ${SOURCES})
