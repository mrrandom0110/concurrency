#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <concurrency/executors/fast/task.hpp>
#include <span>

// Single-Producer / Multi-Consumer Bounded Ring Buffer
namespace executor {
template <size_t Capacity, typename T>
class WorkStealingQueue {
  struct Slot {
    std::atomic<T*> pack_item{nullptr};
  };

  // SPMC

 public:
  bool TryPush(T* item) {
    auto head = head_.load();
    auto tail = tail_.load();
    auto n = tail - head;
    if (n >= Capacity) {
      return false;
    }
    buffer_[tail % uint64_t(Capacity)].pack_item.store(item);
    tail_.store(tail + 1);
    return true;
  }

  void PushMany(std::span<T*> buffer) {
    auto head = head_.load();
    auto tail = tail_.load();
    auto space = tail - head;
    assert(space >= buffer.size());
    for (size_t i = 0; i < buffer.size(); ++i) {
      buffer_[tail % uint64_t(Capacity)].pack_item.store(buffer[i]);
    }
    tail_.store(tail + buffer.size());
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    while (true) {
      auto head = head_.load();
      auto tail = tail_.load();
      if (head == tail) {
        return nullptr;
      }
      auto* item = buffer_[head % uint64_t(Capacity)].pack_item.load(
          std::memory_order_relaxed);
      if (head_.compare_exchange_weak(head, head + 1)) {
        return item;
      }
    }
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> out_buffer) {
    while (true) {
      auto head = head_.load();
      auto tail = tail_.load();
      auto n = tail - head;
      if (n == 0) {
        return 0;
      }
      std::size_t i = 0;
      for (; i < out_buffer.size() && n > 0; ++i, --n) {
        out_buffer[i] =
            buffer_[(head + i) % uint64_t(Capacity)].pack_item.load();
      }
      if (head_.compare_exchange_strong(head, head + i)) {
        return i;
      }
    }
  }

  size_t SizeUpperBound() const {
    auto head = head_.load(std::memory_order_relaxed);
    auto tail = tail_.load(std::memory_order_relaxed);
    return tail - head;
  }

  size_t SizeLowerBound() { return Capacity - SizeUpperBound(); }

 private:
  std::atomic<uint64_t> head_{0};
  std::atomic<uint64_t> tail_{0};
  std::array<Slot, Capacity> buffer_;
};
}  // namespace executor