#pragma once

#include <condition_variable>
#include <queue>

namespace container {
template<typename T>
class ConcurrentQueue {
private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_cond;
public:
  T pop() {
    std::unique_lock<std::mutex> qlock(m_mutex);
    m_cond.wait(qlock, [this] { return !m_queue.empty(); });
    auto item = m_queue.front();
    m_queue.pop();
    return item;
  }

  void pop(T &item) {
    std::unique_lock<std::mutex> qlock(m_mutex);
    m_cond.wait(qlock, [this] { return !m_queue.empty(); });
    item = m_queue.front();
    m_queue.pop();
  }

  void push(const T &item) {
    {
      std::unique_lock<std::mutex> qlock(m_mutex);
      m_queue.push(item);
    }
    m_cond.notify_one();
  }

  void push(T &&item) {
    {
      std::unique_lock<std::mutex> qlock(m_mutex);
      m_queue.push(std::move(item));
    }
    m_cond.notify_one();
  }
};
}