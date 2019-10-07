#pragma once

#include <map>
#include <string>
#include <vector>

namespace gossip {
enum PeerState { ALIVE, ZOMBIE };

class Peer {
private:
  unsigned int _heartbeat = 0;
  PeerState _state = ALIVE;
  std::string _address;
  std::string _id;

public:
  explicit Peer(std::string id, std::string address);
  ~Peer();
  std::string getID() const;
  std::string getAddress() const;
  unsigned int getHeartbeat() const;
  PeerState getState() const;
  void increment();
  void setState(PeerState st);
};

class Members {
private:
  std::map<std::string, Peer> alive;
  std::map<std::string, Peer> dead;
  std::map<std::string, Peer> timers;

public:
  Members();
  bool isAlive(const std::string &id) const;
  bool isDead(const std::string &id) const;
  void increment(const std::string &id);
  void addPeer(Peer);
};

} // namespace gossip
