# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(catch
        GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
        GIT_TAG "v2.9.2"
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        CMAKE_ARGS "-DBUILD_TESTING=OFF"
        INSTALL_COMMAND ""
        )

