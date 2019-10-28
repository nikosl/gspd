#pragma once

#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <vector>

#include <msgpack.hpp>
#include <queue>
#include <ConcurentQueue.hpp>
#include <iostream>

#include "SimpleTimer.hpp"

namespace gossip {

class ConcTimerMgr {
public:
  ConcTimerMgr() = default;
  ~ConcTimerMgr() {
    for (auto &tm:m_timers) {
      tm.second->cancel();
    }
    for (auto &t : m_threads) {
      if (t.second.joinable()) {
        t.second.join();
      }
    }
  }

  void start() {
    is_running = true;
    m_mgr_thread = std::thread(&ConcTimerMgr::process, this);
  }

  void stop() {
    is_running = false;
    m_tasks.push(Task{"", Operation::stop});
  }

  void cancel(const std::string &id) {
    m_tasks.push(Task{id, Operation::cancel});
  }

  void create(const std::string &id, int time, std::function<void()> fn) {
    m_tasks.push(Task{id, fn, time, Operation::create});
  }

private:
  std::thread m_mgr_thread;
  enum class Operation {
    create,
    cancel,
    stop
  };

  struct Task {
    Task(const std::string &t_id, std::function<void()> &t_fn, int t_time, Operation t_op) : id(t_id),
                                                                                             fn(std::move(t_fn)),
                                                                                             time(t_time),
                                                                                             op(t_op) {};

    Task(const std::string &t_id, enum Operation t_op) : id(t_id), fn(), time(), op(t_op) {

    }
    std::string id;
    std::function<void()> fn;
    int time;
    Operation op;
  };

  //std::queue<std::string> m_threads_done;
  container::ConcurrentQueue<Task> m_tasks;
  std::unordered_map<std::string, std::thread> m_threads;
  std::unordered_map<std::string, std::shared_ptr<timer::SimpleTimer>> m_timers;
  std::atomic<bool> is_running{false};

  void process() {
    while (is_running.load()) {
      auto t = m_tasks.pop();
      switch (t.op) {
      case Operation::create:create_timed_task(t.id, t.time, t.fn);
        break;
      case Operation::cancel:cancel_timed_task(t.id);
        break;
      case Operation::stop:is_running.store(false);
        break;
      }
    }
  }

  void create_timed_task(const std::string &id, int time, const std::function<void()> &fn) {
    auto timer(std::make_shared<timer::SimpleTimer>());
    auto[t_it, okt] = m_timers.emplace(id, timer);
    if (!okt) {
      t_it->second->cancel();
      t_it->second = timer;
    }

    auto func = [&]() {
      timer->start_sync(time, fn);
    };

    bool ok;
    std::tie(std::ignore, ok) = m_threads.emplace(id, func);
    if (!ok) {
      auto &t = m_threads.at(id);
      if (t.joinable()) {
        t.join();
      }
      m_threads.erase(id);
      m_threads.emplace(id, func);
    }
  }

  void cancel_timed_task(const std::string &id) {
    if (m_timers.find(id)!=m_timers.cend()) {
      auto &tm = m_timers.at(id);
      tm->cancel();
      auto &t = m_threads.at(id);
      if (t.joinable()) {
        t.join();
      }
      m_threads.erase(id);
      m_timers.erase(id);
    }
  }
};

class Peer {
private:
  std::string id_;
  std::string address_;
  std::chrono::time_point<std::chrono::steady_clock> m_timestamp_;
  unsigned int heartbeat_ = 1;
  mutable std::mutex g_i_mutex;

public:
  Peer(std::string id, std::string address);
  Peer() = default;
  ~Peer() = default;
  Peer(Peer const &other);
  Peer &operator=(Peer const &other);

  std::string get_id() const;
  std::string get_address() const;
  unsigned int get_heartbeat() const;
  void heartbeat(unsigned int i);
  void inc_heartbeat();
  void update_timestamp(int tround);

  std::chrono::time_point<std::chrono::steady_clock> get_timestamp() const;
  friend bool operator>(const Peer &lhs, const Peer &rhs);
  friend bool operator<(const Peer &lhs, const Peer &rhs);
  friend bool operator==(const Peer &lhs, const Peer &rhs);
  friend bool operator!=(const Peer &lhs, const Peer &rhs);
  //friend void swap(Peer &lhs, Peer &rhs);

  friend std::ostream &operator<<(std::ostream &, const Peer &);
  MSGPACK_DEFINE (id_, address_, heartbeat_);

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.String("id");
    writer.String(id_.c_str());

    writer.String("address");
    writer.String(address_.c_str());

    writer.String("heartbeat");
    writer.Uint(heartbeat_);
    writer.EndObject();
  }
};

class MembersTable {
public:
  MembersTable();
  bool is_alive(const std::string &id) const;
  bool is_dead(const std::string &id) const;
  void add_peer(Peer &peer);
  std::shared_ptr<Peer> get_peer(const std::string &id);
  std::shared_ptr<Peer> get_suspect(const std::string &id);
  std::vector<Peer> get_alive_peers() const;
  std::vector<Peer> get_suspected_peers() const;
  int size() const;
  void to_alive(const std::string &id);
  void to_suspected(const std::string &id);

  void cleanup(const std::string &id);

private:
  std::unordered_map<std::string, std::shared_ptr<Peer>> alive_{};
  std::unordered_map<std::string, std::shared_ptr<Peer>> dead_{};

  mutable std::mutex m_members_mutex;
};

class Members {
private:
  std::atomic<bool> cleanup_is_running{true};

  std::unique_ptr<std::thread> t_;
  std::unique_ptr<MembersTable> members_ = std::make_unique<MembersTable>();
  std::string_view me_;

  int tfail_ = 150;
  int tcleanup_ = tfail_*2;
  int tround_ = 150;
public:
  Members();
  ~Members();

  void heartbeat(Peer &peer);
  void add_peer(Peer &peer);
  std::vector<Peer> get_alive_peers() const;
  std::vector<Peer> get_suspected_peers() const;
  std::vector<Peer> get_random_peers(unsigned int k) const;
  void deadline(const std::string &id);
  void cleanup(const std::string &id);
  void set_tfail(int t);
  void set_tclean(int t);
  int get_tround() const;
  void set_tround(int Tround);
  int size() const;
  void gossip();
  void cleanup_task();
  void start_cleanup();
  void stop_cleanup();
  bool is_alive(const std::string &id) const;
  bool is_dead(const std::string &id) const;
  void set_me(std::string_view t_me);
  std::string_view get_me();
  void to_suspected(const std::string &id);
  std::shared_ptr<Peer> get_peer(const std::string &id);
};
} // namespace gossip
