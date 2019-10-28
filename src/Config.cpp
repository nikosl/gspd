#include <algorithm>
#include <sstream>
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

std::string Config::get_my_id() const {
  return my_id_;
}

std::string Config::get_my_address() const {
  return address_;
}

Config::peers_t Config::get_seeds() const {
  return seeds_;
}

bool Config::_set_my_id() {
  auto[val, ok] = _get_env(MY_ID);
  if(ok) {
    my_id_ = std::move(val);
  }
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
    auto pl = split(val, ',');
    for (const auto &p: pl) {
      auto pair = split(p, '=');
      seeds_.emplace_back(std::make_tuple(pair[0], pair[1]));
    }
  }
  return ok;
}

std::tuple<std::string, bool> Config::_get_env(const std::string &t_key) {
  auto ok = false;
  std::string val;
  char *env_p{nullptr};
  env_p = std::getenv(t_key.c_str());
  if (env_p!=nullptr) {
    val = env_p;
    ok = true;
  }
  return std::make_tuple(val, ok);
}

std::vector<std::string> Config::split(const std::string &s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream{s};
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}
} // namespace gossip