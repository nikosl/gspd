#include <memory>
#include <utility>
#include <random>
#include <algorithm>
#include "gossip.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace gossip {

Peer::Peer(std::string peer_id, std::string peer_address)
    : id_(std::move(peer_id)), address_(std::move(peer_address)) {};

std::string Peer::get_address() const { return address_; }

std::string Peer::get_id() const { return id_; }

unsigned int Peer::get_heartbeat() const {
  std::lock_guard<std::mutex> lock(g_i_mutex);
  return heartbeat_;
}

void Peer::heartbeat(unsigned int i) {
  std::lock_guard<std::mutex> lock(g_i_mutex);
  heartbeat_ = i;
}

void Peer::inc_heartbeat() {
  std::lock_guard<std::mutex> lock(g_i_mutex);
  ++heartbeat_;
}

bool operator>(const Peer &lhs, const Peer &rhs) {
  return lhs.heartbeat_ > rhs.heartbeat_;
}

bool operator<(const Peer &lhs, const Peer &rhs) {
  return rhs > lhs;
}

bool operator==(const Peer &lhs, const Peer &rhs) {
  return lhs.id_==rhs.id_;
}

bool operator!=(const Peer &lhs, const Peer &rhs) {
  return !(lhs==rhs);
}

void Peer::update_timestamp(int tround) {
  std::lock_guard<std::mutex> lock(g_i_mutex);
  m_timestamp_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(tround);
}

std::chrono::time_point<std::chrono::steady_clock> Peer::get_timestamp() const {
  std::lock_guard<std::mutex> lock(g_i_mutex);
  return m_timestamp_;
}

Peer &Peer::operator=(Peer const &other) {
  std::unique_lock<std::mutex> lock_this(g_i_mutex, std::defer_lock);
  std::unique_lock<std::mutex> lock_other(other.g_i_mutex, std::defer_lock);
  std::lock(lock_this, lock_other);

  id_ = other.id_;
  address_ = other.address_;
  heartbeat_ = other.heartbeat_;
  m_timestamp_ = other.m_timestamp_;

  return *this;
}

Peer::Peer(Peer const &other) {
  std::unique_lock<std::mutex> lock_other(other.g_i_mutex);
  id_ = other.id_;
  address_ = other.address_;
  heartbeat_ = other.heartbeat_;
  m_timestamp_ = other.m_timestamp_;
}

std::ostream &operator<<(std::ostream &strm, const Peer &peer) {
  return strm << R"("peer":{ "id": )" << peer.id_
              << R"(, "address": )" << "\"" << peer.address_ << "\""
              << R"(, "heartbeat": )" << peer.heartbeat_
              << "}";
}

Members::Members() = default;

Members::~Members() {
  if (t_!=nullptr) {
    cleanup_is_running.store(false);
    if (t_->joinable()) {
      t_->join();
    }
  }
}

void Members::set_tfail(int t) {
  tfail_ = t;
}

void Members::set_tclean(int t) {
  tcleanup_ = t;
}

void Members::deadline(const std::string &id) {
  if (members_->is_alive(id)) {
    spdlog::info("Suspected peer: {}", *members_->get_peer(id));
    auto peer = members_->get_peer(id);
    peer->update_timestamp(tround_);
    members_->to_suspected(id);
  }
}

void Members::cleanup(const std::string &id) {
  if (members_->is_dead(id)) {
    spdlog::info("Remove peer: {}", *members_->get_suspect(id));
    members_->cleanup(id);
  }
}

void Members::heartbeat(Peer &peer) {
  auto id = peer.get_id();
  if (members_->is_alive(id)) {
    auto peer_existing = members_->get_peer(id);
    if (*peer_existing < peer) {
      peer_existing->update_timestamp(tround_);
      peer_existing->heartbeat(peer.get_heartbeat());
    }
  } else if (members_->is_dead(id)) {
    auto peer_existing = members_->get_suspect(id);
    if (*peer_existing < peer) {
      spdlog::info("Heard from suspected peer: {}", peer);
      peer_existing->update_timestamp(tround_);
      peer_existing->heartbeat(peer.get_heartbeat());
      members_->to_alive(id);
    }
  } else {
    spdlog::info("New peer found: {}", peer);
    peer.update_timestamp(tround_);
    members_->add_peer(peer);
  }
}

void MembersTable::to_suspected(const std::string &id) {
  if (is_alive(id)) {
    std::unique_lock<std::mutex> lock(m_members_mutex);
    auto &p = alive_.at(id);
    dead_.insert({id, p});
    alive_.erase(id);
  }
}

void MembersTable::to_alive(const std::string &id) {
  if (is_dead(id)) {
    {
      std::unique_lock<std::mutex> lock(m_members_mutex);
      auto &p = dead_.at(id);
      alive_.insert({id, p});
      dead_.erase(id);
    }
  }
}

bool MembersTable::is_alive(const std::string &id) const {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  return alive_.find(id)!=alive_.cend();
}

bool MembersTable::is_dead(const std::string &id) const {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  return dead_.find(id)!=dead_.cend();
}

std::vector<Peer> MembersTable::get_alive_peers() const {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  std::vector<Peer> v;
  v.reserve(alive_.size());
  for (const auto &p : alive_) {
    v.emplace_back(*p.second);
  }
  return v;
}

int MembersTable::size() const {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  return alive_.size();
}

std::vector<Peer> MembersTable::get_suspected_peers() const {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  std::vector<Peer> v;
  v.reserve(dead_.size());
  for (const auto &p : dead_) {
    v.emplace_back(*p.second);
  }
  return v;
}

void MembersTable::add_peer(Peer &peer) {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  alive_.emplace(peer.get_id(), std::make_shared<Peer>(peer));
}

std::shared_ptr<Peer> MembersTable::get_suspect(const std::string &id) {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  return dead_.at(id);
}
std::shared_ptr<Peer> MembersTable::get_peer(const std::string &id) {
  std::unique_lock<std::mutex> lock(m_members_mutex);
  return alive_.at(id);
}

void MembersTable::cleanup(const std::string &id) {
  if (is_alive(id))
    return;
  std::unique_lock<std::mutex> lock(m_members_mutex);
  dead_.erase(id);
}

MembersTable::MembersTable() = default;

void Members::gossip() {

  //  while(is_running_){
  //  members table

  //  }
}

void Members::cleanup_task() {
  for (const auto &p:members_->get_suspected_peers()) {
    if (p.get_id()==me_)
      continue;

    auto ts = p.get_timestamp() + std::chrono::milliseconds(tcleanup_);
    auto now = (std::chrono::steady_clock::now());
    if (ts < now) {
      cleanup(p.get_id());
    }
  }

  for (auto &p:members_->get_alive_peers()) {
    if (p.get_id()==me_)
      continue;
    auto ts = p.get_timestamp() + std::chrono::milliseconds(tfail_);
    auto now = std::chrono::steady_clock::now();
    if (ts < now) {
      deadline(p.get_id());
    }
  }
}

void Members::start_cleanup() {
  cleanup_is_running.store(true);
  t_ = std::make_unique<std::thread>([this]() {
    while (cleanup_is_running.load()) {
      std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(50));
      cleanup_task();
    }
  });
}

void Members::stop_cleanup() {
  cleanup_is_running.store(false);
  if (t_->joinable()) {
    t_->join();
  }
}

int Members::size() const {
  return members_->size();
}

std::vector<Peer> Members::get_random_peers(unsigned int k) const {
  std::vector<Peer> a = members_->get_alive_peers();
  if (a.empty() || k < 0) {
    return std::vector<Peer>{};
  }
  std::vector<Peer> p;
  p.reserve(k);

  std::random_device rd;
  auto gen = std::mt19937(rd());

  auto c = a.size() - 1;
  auto x = k;
  if (k > c) {
    x = c;
  }

  for (auto i = x; i > 0; --i) {
    std::uniform_int_distribution<int> d(0, c);
    auto n = a[d(gen)];
    if (n.get_id()==me_)
      continue;
    p.push_back(n);
  }
  return p;
}

std::vector<Peer> Members::get_alive_peers() const {
  return members_->get_alive_peers();
}

std::vector<Peer> Members::get_suspected_peers() const {
  return members_->get_suspected_peers();
}

void Members::add_peer(Peer &peer) {
  peer.update_timestamp(tround_);
  members_->add_peer(peer);
}

bool Members::is_dead(const std::string &id) const {
  return members_->is_dead(id);
}

bool Members::is_alive(const std::string &id) const {
  return members_->is_alive(id);
}
void Members::set_me(std::string_view t_me) {
  me_ = t_me;
}

std::string_view Members::get_me() {
  return me_;
}
void Members::to_suspected(const std::string &id) {
  members_->to_suspected(id);
}

std::shared_ptr<Peer> Members::get_peer(const std::string &id) {
  return members_->get_peer(id);
}

int Members::get_tround() const {
  return tround_;
}

void Members::set_tround(int Tround) {
  tround_ = Tround;
}

} // namespace gossip

