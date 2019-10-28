#pragma once
#include <sstream>
#include <vector>
#include "gossip.hpp"

namespace gossip {
class Listener {
public:
  int listen_gossip(int sockfd, char *msg, std::size_t max_size, int cliaddr);
  std::vector<gossip::Peer> deserialize(const char *sbuf, std::size_t size);
  int create_connection(const std::string &addr,const std::string &port);
};
} // namespace gossip