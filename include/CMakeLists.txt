cmake_minimum_required(VERSION 2.8)

include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(mremap HAVE_MREMAP)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE("endian.h" HAVE_ENDIAN_H)
CHECK_INCLUDE_FILE("machine/endian.h" HAVE_MACHINE_ENDIAN_H)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(protostream INTERFACE)
set_property(TARGET protostream APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})
