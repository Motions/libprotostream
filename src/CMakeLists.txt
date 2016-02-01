cmake_minimum_required(VERSION 2.8)

#project(libprotostream)

set (SOURCES
    common.cpp
    stream_reader.cpp
    stream_writer.cpp
)


# static library for internal/test use
add_library(libprotostream_static STATIC ${SOURCES})
add_library(libprotostream SHARED ${SOURCES})
