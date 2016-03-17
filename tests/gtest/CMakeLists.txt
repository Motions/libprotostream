find_package(Threads)
include(ExternalProject)
set(GTEST_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/gtest")
set(GTEST_DIR
        "${GTEST_PREFIX}/src/gtest_external-build")
set(GTEST_INCLUDE_DIR
        "${GTEST_PREFIX}/src/gtest_external/include")
set(GTEST_LIB
        "${GTEST_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(GTEST_MAIN_LIB
        "${GTEST_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}")

file(MAKE_DIRECTORY "${GTEST_INCLUDE_DIR}")
ExternalProject_Add(gtest_external
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG "release-1.7.0"
        INSTALL_COMMAND ""
        PREFIX "${GTEST_PREFIX}")

add_library(gtest IMPORTED STATIC GLOBAL)
set_target_properties(gtest PROPERTIES
        IMPORTED_LOCATION "${GTEST_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

add_library(gtest_main IMPORTED STATIC GLOBAL)
set_target_properties(gtest_main PROPERTIES
        IMPORTED_LOCATION "${GTEST_MAIN_LIB}"
        IMPORTED_LINK_INTERFACE_LIBRARIES "${GTEST_LIB};${CMAKE_THREAD_LIBS_INIT}")

add_dependencies(gtest gtest_external)
add_dependencies(gtest_main gtest_external)