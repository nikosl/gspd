#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <utility>
#include "spdlog/spdlog.h"

namespace timer {
class SimpleTimer {

private:
  std::mutex m_;
  std::condition_variable cv_;
  std::thread t_;
  std::atomic<bool> cancelled_{false};
  std::atomic<bool> threaded_{false};
public:
  SimpleTimer() = default;
  SimpleTimer(const SimpleTimer &) = delete;

  SimpleTimer &operator=(const SimpleTimer &) = delete;

  ~SimpleTimer() {
    if(!cancelled_.load()){
      cancel();
    }
    if (threaded_.load() && t_.joinable()) {
      t_.join();
    }
  }

  template<typename Function>
  void start(int ms, Function fn) {
    std::thread t([=]() {
                    std::unique_lock<std::mutex> lk(m_);
                    spdlog::info("wait for {}ms", ms);
                    cv_.wait_until(lk,
                                   std::chrono::steady_clock::now() + std::chrono::milliseconds(ms),
                                   [this] { return cancelled_.load(); });
                    if (cancelled_.load()) {
                      spdlog::info("Timer stopped");
                      return;
                    }
                    spdlog::info("Execute timer task");
                    fn();
                    lk.unlock();
                  }
    );
    t_ = std::move(t);
  }

  template<typename Function>
  void start_sync(int ms, Function fn) {
    spdlog::info("wait for {}ms", ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    if (cancelled_.load()) {
      spdlog::info("Timer stopped");
      return;
    }
    spdlog::info("Execute timer task");
    fn();
  }

  void cancel() noexcept {
    cancelled_.store(true);
    if(threaded_.load())
      cv_.notify_one();
  }
};

} // namespace timer