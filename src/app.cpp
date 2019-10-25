#include <string>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include "Config.hpp"
#include "gossip.hpp"

int main() {
  gossip::Config config{};

  if (!config.init()) {
    return 1;
  }

  auto me = gossip::Peer{config.get_my_id(), config.get_my_address()};
  auto seeds = config.get_seeds();
  auto members = gossip::Members{};

  for (auto p : seeds) {
    const auto&[id, addr] = p;
    auto n = gossip::Peer{id, addr};
    members.add_peer(n);
  }

  std::thread listener(
      [config] {
        while (true) {

        }
      }
  );

  std::thread sender([config] {
    while (true) {

    }
  });

  listener.join();
  sender.join();
}
