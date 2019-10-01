# Resources:
#  - http://zeromq.org/intro:get-the-software
#  - http://bit.ly/2dK0UBT
#
# Remember to have libtool, pkg-config, build-essential, autoconf, and automake
# installed.

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

SET(CXX "clang++ -stdlib=c++11")

ExternalProject_Add(libzmq
    URL "https://github.com/zeromq/libzmq/releases/download/v4.3.1/zeromq-4.3.1.tar.gz"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendor/libzmq"
    CONFIGURE_COMMAND "${CMAKE_SOURCE_DIR}/vendor/libzmq/configure"
    BUILD_COMMAND make -j4
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
)

set(ZEROMQ_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/zeromq/include PARENT_SCOPE)
set(ZEROMQ_LINK_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/zeromq/.libs PARENT_SCOPE)