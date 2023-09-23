#pragma once

#include <algorithm>
#include <concurrency/executors/fast/task.hpp>
#include <concurrency/utility/intrusive/intrusive_list.hpp>
#include <iostream>
#include <mutex>
#include <span>
#include <string>
#include <type_traits>

namespace executor {
class GlobalQueue {
 public:
  void PushOne(TaskBase* node) {
    std::lock_guard locker{mutex_};
    list_.Push(node);
  }

  TaskBase* PopFront() {
    std::lock_guard locker{mutex_};
    return list_.Pop();
  }

  std::size_t Offload(std::span<TaskBase*> out_buffer) {
    std::lock_guard locker{mutex_};
    std::size_t size = 0;
    for (std::size_t i = 0; i < out_buffer.size(); ++i) {
      if (TaskBase* item = list_.Pop(); item != nullptr) {
        out_buffer[i] = item;
        ++size;
      } else {
        return size;
      }
    }
    return size;
  }

  std::size_t Grab(std::span<TaskBase*> out_buffer, std::size_t workers) {
    std::lock_guard locker{mutex_};
    std::size_t size = list_.Size();
    if (size == 0) {
      return 0;
    }
    std::size_t share = std::max<std::size_t>(size / workers, 1);
    std::size_t to_grab = std::min(out_buffer.size(), share);
    std::size_t count = 0;
    while (to_grab > 0) {
      TaskBase* item = list_.Pop();
      if (item == nullptr) {
        break;
      }
      out_buffer[count++] = item;
      --to_grab;
    }
    return count;
  }

 private:
  std::mutex mutex_;
  utility::IntrusiveList<TaskBase> list_;
};
}  // namespace executor