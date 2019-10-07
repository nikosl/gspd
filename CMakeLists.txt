cmake_minimum_required(VERSION 3.0)
project(gspd)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=libc++11")

set(ZeroMQ_DIR ${CMAKE_CURRENT_BINARY_DIR}/vendor/libzmq)
set(ZMQ_BUILD_TESTS OFF)
set(CPPZMQ_BUILD_TESTS OFF)
add_subdirectory(vendor/libzmq)
add_subdirectory(vendor/cppzmq)
add_subdirectory(vendor/Catch2)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(include)

include_directories(vendor/msgpack-c/include)

#find_package(cppzmq)
add_executable(gspd src/app.cpp)
target_link_libraries(gspd -static pthread cppzmq-static)
