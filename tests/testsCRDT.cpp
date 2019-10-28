#include <catch2/catch.hpp>
#include <crdt.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <iostream>

TEST_CASE("CRDTs GCounter increment", "[crdt]") {
  crdt::GCounter counter{"id0"};
  counter.increment();
  REQUIRE(counter.value()==1);
}

TEST_CASE("CRDTs GCounter merge", "[crdt]") {
  crdt::GCounter counter0{"0"};
  crdt::GCounter counter1{"1"};
  counter0.increment();
  counter1.increment();
  counter0.merge(counter1);
  REQUIRE(counter0.value()==2);
}

TEST_CASE("CRDTs GCounter merge bigger", "[crdt]") {
  crdt::GCounter counter0{"0"};
  crdt::GCounter counter1{"1"};
  crdt::GCounter counter2{"2"};
  counter0.increment();
  counter1.increment();
  counter0.merge(counter1);
  counter1.increment();
  counter0.merge(counter1);
  REQUIRE(counter0.value()==3);
}

TEST_CASE("CRDTs GCounter partial order", "[crdt]") {
  crdt::GCounter counter0{"0"};
  crdt::GCounter counter1{"1"};
  counter0.increment();
  counter1.increment();
  counter0.merge(counter1);
  counter1.merge(counter0);
  REQUIRE(counter0.compare(counter1));
}

TEST_CASE("CRDTs GCounter conflict", "[crdt]") {
  crdt::GCounter counter0{"0"};
  crdt::GCounter counter1{"1"};
  crdt::GCounter counter2{"2"};
  counter0.increment();
  counter1.increment();
  counter0.merge(counter1);
  counter0.merge(counter2);
  counter0.increment();
  counter2.increment();
  counter1.merge(counter2);
  REQUIRE_FALSE(counter0.compare(counter1));
}