#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include <tuple>

namespace gossip {

class Config {
  enum class peerz {
    ID,
    ADDRESS,
  };
  using peers_t = std::vector<std::tuple<std::string_view, std::string_view>>;

public:
  Config();
  ~Config();
  bool init();
private:
  std::string my_id_;
  std::string address_;
  peers_t seeds_;
  bool _set_my_id();
  bool _set_address();
  bool _set_seeds();

  std::tuple<std::string, bool> _get_env(const char *t_key);

  const char *const MY_ID = "MY_ID";
  const char *const MY_ADDRESS = "ADDRESS";
  const char *const SEEDS = "SEEDS";
  std::vector<std::string_view> split_(std::string_view t_str, std::string_view t_delims);
};
} // namespace gossip
