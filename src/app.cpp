#include <thread>
#include <csignal>
#include "Config.hpp"
#include "gossip.hpp"
#include "Client.hpp"
#include "Listener.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

int main() {
  gossip::Config config{};

  if (!config.init()) {
    return -1;
  }

  static std::atomic<bool> is_running{true};

  auto my_id = config.get_my_id();
  auto me = gossip::Peer{my_id, config.get_my_address()};
  auto seeds = config.get_seeds();

  std::shared_ptr<gossip::Members> members = std::make_shared<gossip::Members>();
  members->set_tfail(1000);
  members->set_tclean(2000);
  members->set_me(my_id);
  members->add_peer(me);

  for (const auto &p : seeds) {
    const auto&[id, addr] = p;
    auto n = gossip::Peer{id, addr};
    spdlog::info("Add seed node {}", n);
    members->add_peer(n);
  }

  std::thread listener(
      [&] {
        auto a = gossip::Config::split(config.get_my_address(), ':');
        auto addr = a[0];
        auto port = a[1];
        gossip::Listener server{};
        auto sockfd = server.create_connection(addr, port);
        while (is_running) {
          char buf[2048];
          auto s = server.listen_gossip(sockfd, buf, 2048, 0);
          auto msg = server.deserialize(buf, s);
          for (auto &p:msg) {
            members->heartbeat(p);
          }
        }
        ::close(sockfd);
      }
  );

  std::thread sender([&] {
    spdlog::info("Initial run, send broadcast message id:{}", my_id);
    auto me = members->get_peer(my_id);
    gossip::Client client{};
    me->inc_heartbeat();
    {
      auto table = members->get_alive_peers();
      msgpack::sbuffer sbuf;
      auto s = client.serialize(sbuf, table);
      for (const auto &p:members->get_alive_peers()) {
        auto addr = gossip::Config::split(p.get_address(), ':');
        client.send_members(sbuf.data(), s, addr[0], addr[1]);
      }
    }
    members->start_cleanup();
    while (is_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      me->inc_heartbeat();
      auto k = members->get_random_peers(3);
      auto table = members->get_alive_peers();
      msgpack::sbuffer sbuf;
      auto s = client.serialize(sbuf, table);
      for (const auto &p: k) {
        auto addr = gossip::Config::split(p.get_address(), ':');
        client.send_members(sbuf.data(), s, std::string(addr[0]), std::string(addr[1]));
      }
    }
    members->stop_cleanup();
  });

  struct sigaction sigIntHandler{};

  void (*sig_handler)(int) = [](int s) {
    is_running.store(false);
    exit(0);
  };
  sigIntHandler.sa_handler = sig_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);

  listener.join();
  sender.join();
}
