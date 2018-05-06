cmake_minimum_required(VERSION 3.10)
project(nibac)
set(CMAKE_CXX_FLAGS "-std=c++17")

include(ExternalProject)
set(LIBS_EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs_extern)
set(NAUTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src_extern/nauty22)
ExternalProject_Add(nauty22
        SOURCE_DIR ${NAUTY_DIR}
        CONFIGURE_COMMAND ${NAUTY_DIR}/configure --prefix=${LIBS_EXTERN_DIR}
        BUILD_COMMAND ${MAKE}
        BUILD_IN_SOURCE 1)

