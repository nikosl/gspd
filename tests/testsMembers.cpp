#include <catch2/catch.hpp>
#include <msgpack.hpp>
#include <iostream>
#include "gossip.hpp"

TEST_CASE("Members should be handled by gossip", "[members]") {
  SECTION("Member is not known") {
    gossip::Members members{};
    REQUIRE_FALSE((members.is_alive("12345") && members.is_dead("12345")));
  }

  SECTION("Member is alive") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    members.add_peer(peer);
    REQUIRE(members.is_alive("123"));
    REQUIRE_FALSE(members.is_dead("123"));
  }

  SECTION("Increment heartbeat") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    members.add_peer(peer);
    peer.inc_heartbeat();
    peer.inc_heartbeat();
    members.heartbeat(peer);
    REQUIRE(peer.get_heartbeat()==3);
  }

  SECTION("Receive new peer") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    members.heartbeat(peer);
    REQUIRE(members.is_alive("123"));
    REQUIRE_FALSE(members.is_dead("123"));
  }

  SECTION("heartbeat is less or equal") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    peer.inc_heartbeat();
    peer2.inc_heartbeat();
    members.heartbeat(peer);
    members.heartbeat(peer2);
    REQUIRE(peer > peer2);
  }

  SECTION("heartbeat is equal") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    peer2.inc_heartbeat();
    members.heartbeat(peer);
    members.heartbeat(peer2);
    REQUIRE(peer.get_heartbeat()==2);
  }

  SECTION("Move to dead after timeout") {
    gossip::Members members{};
    members.start_cleanup();
    members.set_tfail(50);
    members.set_tclean(150);
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    members.heartbeat(peer);
    REQUIRE(members.is_alive(peer.get_id()));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    members.stop_cleanup();
    REQUIRE(members.is_dead(peer.get_id()));
    REQUIRE_FALSE(members.is_alive(peer.get_id()));
  }

  SECTION("receive message for suspect with the same heartbeat") {
    gossip::Members members{};
    members.set_tfail(50);
    members.set_tclean(150);
    members.start_cleanup();
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    peer2.inc_heartbeat();
    members.heartbeat(peer);
    std::string id = peer.get_id();
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    members.stop_cleanup();
    REQUIRE_FALSE(members.is_alive(id));
    members.heartbeat(peer2);
    REQUIRE_FALSE(members.is_alive(id));
  }

  SECTION("receive message for suspect with larger heartbeat") {
    gossip::Members members{};
    members.set_tfail(50);
    members.set_tclean(150);
    members.start_cleanup();
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    peer2.heartbeat(5);
    members.heartbeat(peer);
    std::string id = peer.get_id();
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    REQUIRE(members.is_dead(id));
    members.heartbeat(peer2);
    members.stop_cleanup();
    REQUIRE(members.is_alive(id));
  }

  SECTION("cleanup dead after timeout") {
    gossip::Members members{};
    members.set_tfail(50);
    members.set_tclean(70);
    members.start_cleanup();
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    members.heartbeat(peer);
    std::string id = peer.get_id();
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    REQUIRE_FALSE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    members.stop_cleanup();
    REQUIRE_FALSE(members.is_dead(id));
  }

  SECTION("timer should not be triggered when heartbeat received") {
    gossip::Members members{};
    members.set_tfail(50);
    members.set_tclean(150);
    members.start_cleanup();
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    gossip::Peer peer3{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    members.heartbeat(peer);
    std::string id = peer.get_id();
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    peer2.heartbeat(3);
    members.heartbeat(peer2);
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    peer3.heartbeat(4);
    members.heartbeat(peer3);
    members.stop_cleanup();
    REQUIRE(members.is_alive(id));
  }

  SECTION("timer cleanup should not be triggered when heartbeat received") {
    gossip::Members members{};
    members.set_tfail(50);
    members.set_tclean(150);
    members.start_cleanup();
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"123", "127.0.0.1:8080"};
    gossip::Peer peer3{"123", "127.0.0.1:8080"};
    peer.inc_heartbeat();
    members.heartbeat(peer);
    std::string id = peer.get_id();
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    peer2.heartbeat(3);
    members.heartbeat(peer2);
    REQUIRE(members.is_alive(id));
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    peer3.heartbeat(4);
    members.heartbeat(peer3);
    members.stop_cleanup();
    REQUIRE(members.is_alive(id));
  }

  SECTION("Get all peers should return a list of only alive peers") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"456", "127.0.0.1:8080"};
    gossip::Peer peer3{"789", "127.0.0.1:8080"};

    std::vector<gossip::Peer> expected{peer, peer2};

    members.add_peer(peer);
    members.add_peer(peer2);
    members.add_peer(peer3);

    members.to_suspected(peer3.get_id());

    auto actual = members.get_alive_peers();

    REQUIRE(expected.size()==members.size());
    REQUIRE(std::equal(expected.cbegin(), expected.cend(), actual.cbegin()));
  }

  SECTION("Get k numbers of random peers") {
    gossip::Members members{};
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"456", "127.0.0.1:8080"};
    gossip::Peer peer3{"789", "127.0.0.1:8080"};
    gossip::Peer peer4{"823", "127.0.0.1:8080"};
    gossip::Peer peer5{"956", "127.0.0.1:8080"};
    gossip::Peer peer6{"1089", "127.0.0.1:8080"};

    std::vector<gossip::Peer> expected{peer, peer2};

    members.add_peer(peer);
    members.add_peer(peer2);
    members.add_peer(peer3);
    members.add_peer(peer4);
    members.add_peer(peer5);
    members.add_peer(peer6);

    auto actual = members.get_random_peers(2);

    auto res = std::search(actual.begin(), actual.end(), expected.begin(), expected.end(),
                           [](gossip::Peer &l, gossip::Peer &r) { return l==r; });
    REQUIRE(res!=expected.end());
  }

  SECTION("deserialize Peer") {
    gossip::Peer peer{"123", "127.0.0.1:8080"};
    gossip::Peer peer2{"456", "127.0.0.1:8081"};
    gossip::Peer peer3{"789", "127.0.0.1:8082"};

    peer.heartbeat(1);
    peer2.heartbeat(10);
    peer3.heartbeat(100);

    std::vector<gossip::Peer> vec;
    vec.push_back(peer);
    vec.push_back(peer2);
    vec.push_back(peer3);

    // serialize
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vec);

    msgpack::object_handle oh =
        msgpack::unpack(sbuf.data(), sbuf.size());

    msgpack::object obj = oh.get();

    // convert object to Peer
    std::vector<gossip::Peer> rvec;
    obj.convert(rvec);

    REQUIRE(std::equal(vec.cbegin(), vec.cend(), rvec.cbegin(),
                       [](const gossip::Peer &lhs, const gossip::Peer &rhs) {
                         return (lhs.get_id()==rhs.get_id()
                             && lhs.get_address()==rhs.get_address()
                             && lhs.get_heartbeat()==rhs.get_heartbeat());
                       }));
  }
}

TEST_CASE("timer manager", "[timermgt]") {
  gossip::Members members{};
  members.set_tfail(50);
  members.set_tfail(150);
  gossip::Peer peer{"123", "127.0.0.1:8080"};
  peer.inc_heartbeat();
  members.heartbeat(peer);
  REQUIRE(members.is_alive(peer.get_id()));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  members.cleanup_task();
  REQUIRE(members.is_dead(peer.get_id()));
  REQUIRE_FALSE(members.is_alive(peer.get_id()));
}

TEST_CASE("timer manager thread", "[timermgt]") {
  gossip::Members members{};
  members.set_tfail(50);
  members.set_tfail(250);
  members.cleanup_task();
  gossip::Peer peer{"123", "127.0.0.1:8080"};
  peer.inc_heartbeat();
  members.heartbeat(peer);
  REQUIRE(members.is_alive(peer.get_id()));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  REQUIRE(members.is_dead(peer.get_id()));
  REQUIRE_FALSE(members.is_alive(peer.get_id()));
}

TEST_CASE("receive message for suspect with larger heartbeat") {
  gossip::Members members{};
  members.set_tfail(50);
  members.set_tclean(150);
  // members.start_cleanup();
  gossip::Peer peer{"123", "127.0.0.1:8080"};
  peer.inc_heartbeat();
  gossip::Peer peer2{"123", "127.0.0.1:8080"};
  peer2.heartbeat(4);
  members.heartbeat(peer);
  std::string id = peer.get_id();
  REQUIRE(members.is_alive(id));
  std::this_thread::sleep_for(std::chrono::milliseconds(70));
  members.deadline(id);
  REQUIRE(members.is_dead(id));
  members.heartbeat(peer2);
  //members.stop_cleanup();
  REQUIRE(members.is_alive(id));
}

TEST_CASE("receive message for suspect with larger heartbeat multithread") {
  gossip::Members members{};
  members.set_tfail(50);
  members.set_tclean(150);
  members.start_cleanup();
  gossip::Peer peer{"123", "127.0.0.1:8080"};
  peer.inc_heartbeat();
  gossip::Peer peer2{"123", "127.0.0.1:8080"};
  peer2.heartbeat(4);
  members.heartbeat(peer);
  std::thread t1([&]() {
    members.heartbeat(peer);
  });
  std::string id = peer.get_id();
  REQUIRE(members.is_alive(id));
  std::this_thread::sleep_for(std::chrono::milliseconds(70));
  //members.deadline(id);
  REQUIRE(members.is_dead(id));
  std::thread t([&]() {
    members.heartbeat(peer2);
  });
  t.join();
  t1.join();
  members.stop_cleanup();
  REQUIRE(members.is_alive(id));
}

TEST_CASE("Get k numbers of random peers sa") {
  gossip::Peer peer{"123", "127.0.0.1:8080"};
  gossip::Peer peer2{"456", "127.0.0.1:8080"};
  gossip::Peer peer3{"789", "127.0.0.1:8080"};
  gossip::Peer peer4{"823", "127.0.0.1:8080"};
  gossip::Peer peer5{"956", "127.0.0.1:8080"};
  gossip::Peer peer6{"1089", "127.0.0.1:8080"};

  gossip::Members members{};
  members.add_peer(peer);
  members.add_peer(peer2);
  members.add_peer(peer3);
  members.add_peer(peer4);
  members.add_peer(peer5);
  members.add_peer(peer6);

  auto actual = members.get_random_peers(5);
  REQUIRE_FALSE(actual.empty());
}