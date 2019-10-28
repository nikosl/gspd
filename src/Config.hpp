#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include <tuple>

namespace gossip {

class Config {
  using peers_t = std::vector<std::tuple<std::string, std::string>>;

public:
  Config();
  ~Config();
  enum class peerz {
    ID,
    ADDRESS,
  };
  static std::vector<std::string> split(const std::string &s, char delimiter = ',');

  [[nodiscard]] bool init();
  std::string get_my_id() const;
  std::string get_my_address() const;
  peers_t get_seeds() const;
private:
  const std::string MY_ID{"MY_ID"};
  const std::string MY_ADDRESS{ "ADDRESS"};
  const std::string SEEDS{"SEEDS"};

  std::string my_id_{};
  std::string address_{};
  peers_t seeds_;
  bool _set_my_id();
  bool _set_address();
  bool _set_seeds();

  std::tuple<std::string, bool> _get_env(const std::string &t_key);
};
} // namespace gossip
