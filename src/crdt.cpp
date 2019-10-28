#include <numeric>
#include <map>
#include <utility>
#include "crdt.hpp"

crdt::GCounter::GCounter(const std::string &t_my_id) : my_id_(t_my_id) {
  payload_.emplace(my_id_, 0);
}

void crdt::GCounter::increment() {
  payload_[my_id_]++;
}

int crdt::GCounter::value() const {
  return std::accumulate(payload_.cbegin(),
                         payload_.cend(),
                         0,
                         [](int value, const std::map<std::string, int>::value_type &p) { return value + p.second; }
  );
}

std::unordered_map<std::string, int> crdt::GCounter::payload() const {
  return payload_;
}

bool crdt::GCounter::merge(const crdt::GCounter &other) {
  bool merged{false};
  auto p = other.payload();
  for (const auto &i:p) {
    auto y = i.second;
    if (payload_.find(i.first)!=payload_.cend()) {
      auto x = payload_[i.first];
      payload_[i.first] = x <= y ? y : x;
      merged = true;
    } else {
      payload_.emplace(i.first, y);
      merged = true;
    }
  }
  return merged;
}

bool crdt::GCounter::compare(const crdt::GCounter &other) {
  auto partial{true};
  if (payload_.size()!=other.payload().size())
    return false;
  for (const auto &i: other.payload()) {
    if (payload_.find(i.first)==payload_.cend())
      return false;
    auto x = payload_[i.first];
    auto y = i.second;
    if (y < x) {
      return false;
    }
  }
  return partial;
}

void crdt::GCounter::set_payload(std::unordered_map<std::string, int> payload) {
  payload_ = std::move(payload);
}




