#include <catch2/catch.hpp>
#include <Client.hpp>
#include <Listener.hpp>

TEST_CASE("Send a peer message to listener", "[client]"){
  std::atomic<bool> started{false};
  std::atomic<bool> is_set{false};

  std::vector<gossip::Peer> actual;
  std::mutex m_;
  std::condition_variable cv_;

  std::thread l([&]{
    gossip::Listener server;
    auto sockfd = server.create_connection("127.0.0.1", "5001");
    char buf[1024];
    started.store(true);
    auto s = server.listen_gossip(sockfd, buf, 1024, 0);
    auto smsg = server.deserialize(buf, s);
    std::unique_lock<std::mutex> lock(m_);
    actual = smsg;
    is_set.store(true);
    lock.unlock();
    cv_.notify_one();
  });

  gossip::Client client{};
  std::vector<gossip::Peer> peers;
  auto p = gossip::Peer{"123", "127.0.0.1:5000"};
  p.inc_heartbeat();
  peers.emplace_back(p);

  msgpack::sbuffer ss;
  auto s = client.serialize(ss, peers);
  while(!started){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  auto r = client.send_members(ss.data(), s, "127.0.0.1", "5001");
  REQUIRE(r == 0);

  std::unique_lock<std::mutex> lk(m_);
  cv_.wait(lk, [&]{return is_set.load();});
  lk.unlock();
  l.join();
  REQUIRE(peers == actual);
}

