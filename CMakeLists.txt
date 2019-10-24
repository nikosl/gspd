cmake_minimum_required(VERSION 3.0)
project(gspd LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wall -pedantic")
#san  -O1 -fno-omit-frame-pointer -fsanitize=undefined -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fsanitize=address -fPIE -pie -g
#set(ZeroMQ_DIR ${CMAKE_CURRENT_BINARY_DIR}/vendor/libzmq)
set(ZMQ_BUILD_TESTS OFF)
set(CPPZMQ_BUILD_TESTS OFF)

add_subdirectory(vendor/libzmq)
add_subdirectory(vendor/cppzmq)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)
include_directories(include)

include_directories(vendor/msgpack-c/include)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

find_package(cppzmq)
add_executable(gspd src/app.cpp src/gossip.cpp src/gossip.hpp include/SimpleTimer.hpp include/ConcurentQueue.hpp src/Config.cpp src/Config.hpp src/Client.cpp src/Client.hpp src/Listener.cpp src/Listener.hpp)
target_link_libraries(gspd -static pthread cppzmq-static spdlog::spdlog_header_only)

find_package(Catch2 REQUIRED)
add_executable(tests tests/tests-main.cpp tests/tests-members.cpp src/gossip.cpp tests/tests-SimpleTimer.cpp include/SimpleTimer.hpp tests/tests-ConcurentQueue.cpp)
target_link_libraries(tests Catch2::Catch2 pthread)

include(CTest)
include(Catch)
catch_discover_tests(tests)