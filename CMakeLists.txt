cmake_minimum_required(VERSION 3.0)
project(gspd)

set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(vendor/catch2)
add_subdirectory(vendor/zeromq)
add_subdirectory(vendor/zeromqcpp)
add_subdirectory(vendor/msgpack-c)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(include)
#include_directories(${CATCH_INCLUDE_DIRS})
include_directories(${ZEROMQCPP_INCLUDE_DIRS})
include_directories(${ZEROMQ_INCLUDE_DIRS})
include_directories(${MSGPACKC_INCLUDE_DIRS})

link_directories(${ZEROMQ_LINK_DIRS})

set(GSPD_LIBRARY_DEPENDENCIES
        zmq
        )
add_executable(gspd src/app.cpp)
target_link_libraries(gspd -static ${GSPD_LIBRARY_DEPENDENCIES})
