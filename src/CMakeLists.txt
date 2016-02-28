cmake_minimum_required(VERSION 2.8)

#project(libprotostream)

set(SOURCES
        mmap_backend.cpp
        )


# static library for internal/test use
add_library(protostream_static STATIC ${SOURCES})
add_library(protostream SHARED ${SOURCES} utils.h)
