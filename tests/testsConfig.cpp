#include <catch2/catch.hpp>
#include <stdlib.h> //setenv
#include "Config.hpp"

TEST_CASE("Configuration initialization", "[config]") {
  const std::string ADDRESS = "127.0.0.1:5000";
  const std::string MY_ID = "1234";
  std::vector<std::tuple<std::string, std::string>> expected_seeds{
    std::make_tuple("1","127.0.0.1:5001"),
    std::make_tuple("2","127.0.0.1:5002")
  };

  gossip::Config config{};
  ::setenv("MY_ID", MY_ID.c_str(), 1);
  ::setenv("ADDRESS", ADDRESS.c_str(), 1);
  ::setenv("SEEDS", "1=127.0.0.1:5001,2=127.0.0.1:5002", 1);

  REQUIRE(config.init());
  REQUIRE(config.get_my_address() == ADDRESS);
  REQUIRE(config.get_my_id() == MY_ID);
  REQUIRE(config.get_seeds() == expected_seeds);
}

TEST_CASE("Configuration initialization with undeclared env vars", "[config]") {
  gossip::Config config{};

  ::setenv("ADDRESS", "127.0.0.1:5000", 1);
  ::setenv("SEEDS", "1=127.0.0.1:5001,2=127.0.0.1:5002", 1);

  REQUIRE_FALSE(config.init());
  REQUIRE(config.get_my_address().empty());
  REQUIRE(config.get_my_id().empty());
  REQUIRE(config.get_seeds().empty());
}