#pragma once
#include <string>
#include <map>
#include <vector>

namespace gossip {
enum PeerState {
  alive,
  zombie
};

class Peer {
 private:
  int64 heartbeat;
  PeerState state;
  std::string address;
  std::string id;

 public:
  Peer(const &
  std::string id,
  const &
  std::string address
  );

  const&
  std::string getID() const;
  const&
  std::string getAddress() const;
  int64 getHeartbeat() const;
  PEER_STATE getState()
  void increment();
};

class Members {
 private:
  std::map<std::string, Peer> alive;
  std::map<std::string, Peer> dead;
  std::map<std::string, Peer> timers;
 public:
  Members();
  bool isAlive(std::string id) const;
  bool isDead(std::string id) const;
  void increment(std::string id);
  void addPeer(Peer);
};

} // namespace gossip
