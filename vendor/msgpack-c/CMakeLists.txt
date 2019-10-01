# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(msgpackc
        GIT_REPOSITORY "https://github.com/msgpack/msgpack-c.git"
        GIT_TAG "master"
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

set(MSGPACKC_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include PARENT_SCOPE)
