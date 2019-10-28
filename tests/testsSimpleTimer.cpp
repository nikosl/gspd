#include <catch2/catch.hpp>
#include <SimpleTimer.hpp>
#include <iostream>
#include <sstream>

TEST_CASE("Simple timer trigger test", "[timer]") {
  timer::SimpleTimer timer;
  std::atomic<bool> triggered{false};
  auto start = std::chrono::high_resolution_clock::now();
  std::function<void()> fn = [&]() {
    triggered.store(true);
    auto now = std::chrono::high_resolution_clock::now();
    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    std::stringstream ss;
    ss << msecs.count();
    std::cout << "elapsed " << ss.str() << "s\t: ";
  };
  timer.start(100, fn);
  std::this_thread::sleep_for(
      std::chrono::milliseconds(150));
  REQUIRE(triggered.load());
}

TEST_CASE("Simple timer trigger test canceled", "[timer]") {
  timer::SimpleTimer timer;
  std::atomic<bool> triggered{false};
  std::function<void()> fn = [&triggered]() {
    triggered.store(true);
  };
  timer.cancel();
  timer.start(10, fn);
  std::this_thread::sleep_for(
      std::chrono::milliseconds(11));
  REQUIRE(!triggered.load());
}

TEST_CASE("Simple timer trigger test canceled before completion", "[timer]") {
  timer::SimpleTimer timer;
  std::atomic<bool> triggered{false};
  std::function<void()> fn = [&triggered]() {
    triggered.store(true);
  };
  timer.start(100, fn);

  std::this_thread::sleep_for(
      std::chrono::milliseconds(20));
  timer.cancel();
  REQUIRE(!triggered.load());
}

TEST_CASE("Simple timer cleaned", "[timer]") {
  timer::SimpleTimer timer;
  std::atomic<bool> triggered{false};
  std::function<void()> fn = [&triggered]() {
    triggered.store(true);
  };
  timer.start(100, fn);
  REQUIRE(!triggered.load());
}
