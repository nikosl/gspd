#include <catch2/catch.hpp>
#include "ConcurentQueue.hpp"

TEST_CASE("Put and remove an element from the queue", "[ConcurrentQueue]") {
  container::ConcurrentQueue<int> q{};
  q.push(1);
  REQUIRE(q.pop() == 1);
}

