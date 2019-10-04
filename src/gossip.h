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
  unsigned int heartbeat;
  PeerState state;
  std::string address;
  std::string id;

 public:
  Peer(std::string id, std::string address);

  std::string getID() const;
  std::string getAddress() const;
  unsigned int getHeartbeat() const;
  PeerState getState() const;
  void increment();
};

class Members {
 private:
  std::map<std::string, Peer> alive;
  std::map<std::string, Peer> dead;
  std::map<std::string, Peer> timers;

 public:
  Members();
  bool isAlive(const std::string& id) const;
  bool isDead(const std::string& id) const;
  void increment(const std::string& id);
  void addPeer(Peer);
};

} // namespace gossip
