#include <algorithm>
#include "Config.hpp"

namespace gossip {

Config::Config() = default;
Config::~Config() = default;

bool Config::init() {
  bool ok{false};
  ok = _set_my_id()
      && _set_address()
      && _set_seeds();
  return ok;
}

std::string Config::get_my_id(){
    return my_id_;
}

std::string Config::get_my_address(){
    return address_;
}

Config::peers_t Config::get_seeds(){
    return seeds_;
}

bool Config::_set_my_id() {
  auto[val, ok] = _get_env(MY_ID);
  my_id_ = val;
  return ok;
}

bool Config::_set_address() {
  auto[val, ok] = _get_env(MY_ADDRESS);
  address_ = val;
  return ok;
}

bool Config::_set_seeds() {
  auto[val, ok] = _get_env(SEEDS);
  if (ok) {
    auto pl = split_(val, ",");
    for (auto p: pl) {
      auto pair = split_(p, "=");
      seeds_.emplace_back(std::make_tuple(pair[0], pair[1]));
    }
  }
  return ok;
}

std::tuple<std::string, bool> Config::_get_env(const char *t_key) {
  auto ok = false;
  auto val = "";
  if (auto env_p = std::getenv(t_key)) {
    val = env_p;
    ok = true;
  }
  return std::make_tuple(val, ok);
}

std::vector<std::string_view> Config::split_(std::string_view t_str, std::string_view t_delims = ",") {
  std::vector<std::string_view> output;
  auto first = t_str.begin();
  while (first!=t_str.end()) {
    const auto second = std::find_first_of(first,
                                           std::cend(t_str),
                                           std::cbegin(t_delims),
                                           std::cend(t_delims));
    if (first!=second) {
      output.emplace_back(t_str.substr(
          std::distance(t_str.begin(), first),
          std::distance(first, second))
      );
    }

    if (second==t_str.end())
      break;

    first = std::next(second);
  }
  return output;
}
} // namespace gossip