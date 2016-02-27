cmake_minimum_required(VERSION 2.8)

#project(libprotostream)

set (SOURCES
    common.cpp
    stream_reader.cpp
    stream_writer.cpp
    mmap_reader.cpp
    libprotostream_c.cpp
)


add_library(protostream STATIC ${SOURCES})
add_library(protostream_dyn SHARED ${SOURCES})
