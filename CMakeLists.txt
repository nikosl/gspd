cmake_minimum_required(VERSION 3.0)
project(gspd LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wall -pedantic")
#san  -O1 -fno-omit-frame-pointer -fsanitize=undefined -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fsanitize=address -fPIE -pie -g

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)
include_directories(include)

include_directories(vendor/msgpack-c/include)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

add_executable(gspd src/app.cpp src/gossip.cpp src/gossip.hpp include/SimpleTimer.hpp
        include/ConcurentQueue.hpp src/Config.cpp src/Config.hpp
        src/Client.cpp src/Client.hpp
        src/Listener.cpp src/Listener.hpp)
target_link_libraries(gspd spdlog::spdlog_header_only)

find_package(Catch2 REQUIRED)
add_executable(tests tests/testsMain.cpp tests/testsMembers.cpp src/gossip.cpp
        tests/testsSimpleTimer.cpp include/SimpleTimer.hpp
        tests/testsConcurentQueue.cpp
        tests/testsConfig.cpp src/Config.cpp
        tests/testsClient.cpp src/Client.cpp
        src/Listener.cpp)
target_link_libraries(tests Catch2::Catch2 pthread)

include(CTest)
include(Catch)
catch_discover_tests(tests)