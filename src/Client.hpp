#pragma once
#include <msgpack.hpp>
#include "gossip.hpp"

namespace gossip {
class Client {
public:
  std::size_t serialize(msgpack::sbuffer &sbuf, const std::vector<gossip::Peer> &peers);
  int send_members(const char *msg, std::size_t size, const std::string &ip, const std::string &port);
};
} // namespace gossip
