# CMakeLists.txt
#
# By Sebastian Raaphorst, 2003 - 2018.

cmake_minimum_required(VERSION 3.10)
project(nibac)
set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/src")
add_subdirectory(src)

# *** Versioning ***
set(NIBAC_MAJOR_VERSION=0)
set(NIBAC_MINOR_VERSION=2)
set(NIBAC_MAINTENANCE_VERSION=1)

configure_file(
        "${PROJECT_SOURCE_DIR}/src/common.h.in"
        "${PROJECT_BINARY_DIR}/src/common.h"
)


# *** nauty bundle ***
include(ExternalProject)
set(EXTERN_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")
set(NAUTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src_extern/nauty22")
ExternalProject_Add(nauty22
        SOURCE_DIR "${NAUTY_DIR}"
        CONFIGURE_COMMAND "${NAUTY_DIR}/configure" --prefix=${EXTERN_OUT_DIR}
        BUILD_COMMAND ${MAKE}
        BUILD_IN_SOURCE 1)

