#include "Client.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace gossip {

int Client::send_members(const char *msg, size_t size, const std::string &ip, const std::string &port) {
  sockaddr_in servaddr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    spdlog::error("cannot open socket");
    return -1;
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
  servaddr.sin_port = htons(stoi(port));
  if (sendto(fd, msg, size, 0,
             (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    spdlog::error("cannot send message");
    close(fd);
    return -2;
  }
  close(fd);
  return 0;
}

std::size_t Client::serialize(msgpack::sbuffer &sbuf, const std::vector<gossip::Peer> &peers) {
  msgpack::pack(sbuf, peers);
  return sbuf.size();
}
} // namespace gossip