#include "Listener.hpp"
namespace gossip {

int Listener::listen_gossip(int sockfd, char *msg, size_t max_size, int cliaddr) {
  int len{0}, n{0};
  n = ::recvfrom(sockfd, msg, max_size,
                 MSG_WAITALL, reinterpret_cast<sockaddr *>(&cliaddr),
                 reinterpret_cast<socklen_t *>(&len));
  return n;
}

std::vector<gossip::Peer> Listener::deserialize(const char *sbuf, size_t size) {
  msgpack::object_handle oh =
      msgpack::unpack(sbuf, size);
  msgpack::object obj = oh.get();

// convert object to Peer
  std::vector<gossip::Peer> rvec;
  obj.convert(rvec);
  return rvec;
}

int Listener::create_connection(const std::string &addr, const std::string &port) {
  struct ::sockaddr_in servaddr{}, cliaddr{};

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    spdlog::error("socket creation failed");
    exit(EXIT_FAILURE);
  }

  bzero(&servaddr, sizeof(servaddr));
  bzero(&cliaddr, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = inet_addr(addr.c_str());
  servaddr.sin_port = htons(std::stoi(port));

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *) &servaddr,
           sizeof(servaddr)) < 0) {
    spdlog::error("bind failed");
    exit(EXIT_FAILURE);
  }
  return sockfd;
}

} // namespace gossip