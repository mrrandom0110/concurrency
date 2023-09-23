#include "worker.hpp"

#include "scheduler.hpp"

namespace executor {
thread_local Worker* Worker::current = nullptr;

Worker::Worker(Scheduler& host, size_t index) : host_(host), index_(index) {}

TaskBase* Worker::TryPickTask() {
  if (iter_ % 61 == 0) {
    if (TaskBase* next = TryPickTaskFromGlobalQueue(); next != nullptr) {
      // throw std::runtime_error("AAA");
      return next;
    }
  }

  if (TaskBase* next = TryPickTaskFromLifoSlot(); next != nullptr) {
    // throw std::runtime_error("AAA");
    return next;
  }

  if (TaskBase* next = local_queue_.TryPop(); next != nullptr) {
    // throw std::runtime_error("CC");

    return next;
  }

  if (TaskBase* next = GrabTasksFromGlobalQueue(); next != nullptr) {
    // throw std::runtime_error("DDD");

    return next;
  }

  // TODO(erickurbanov) Here start spinning. Waiting for work.

  return nullptr;
}

void Worker::Work() {
  current = this;
  while (TaskBase* next = TryPickTask()) {
    if (next) {
      next->Run();
    }
  }
}

void Worker::LocalPush(TaskBase* task, SchedulerHint hint) {
  if (hint == SchedulerHint::Next) {
    PushToLifoSlot(task);
  } else {
    PushToLocalQueue(task);
  }
}

void Worker::PushToLocalQueue(TaskBase* task) {
  if (!local_queue_.TryPush(task)) {
    OffloadTasksToGlobalQueue(task);
  }
}

void Worker::Start() {
  thread_.emplace(std::thread([this] { Work(); }));
}

void Worker::Join() { thread_->join(); }

void Worker::OffloadTasksToGlobalQueue(TaskBase* overflow) {
  size_t to_offload = local_queue_.SizeUpperBound() / 2 + 1;
  size_t batch_size = local_queue_.Grab({buffer_, to_offload});
  buffer_[batch_size++] = overflow;
  host_.global_queue_.Offload({buffer_, batch_size});
}

TaskBase* Worker::GrabTasksFromGlobalQueue() {
  size_t space = local_queue_.SizeLowerBound();
  size_t to_grab = std::min<std::size_t>((space / 2 + 1), 48);
  size_t grab_count =
      host_.global_queue_.Grab({buffer_, to_grab}, host_.threads_);
  TaskBase* next = nullptr;
  if (grab_count > 0) {
    next = buffer_[0];
    local_queue_.PushMany({buffer_ + 1, grab_count - 1});
  }
  return next;
}

TaskBase* Worker::TryPickTaskFromGlobalQueue() {
  TaskBase* next = host_.global_queue_.PopFront();
  return next;
}

TaskBase* Worker::TryPickTaskFromLifoSlot() {
  static const size_t kLifoStepsLimit = 5;
  TaskBase* next = std::exchange(lifo_slot_, nullptr);
  if (next == nullptr) {
    return nullptr;
  }
  if (++lifo_steps_ > kLifoStepsLimit) {
    lifo_steps_ = 0;
    PushToLocalQueue(next);
    return nullptr;
  } else {
    return next;
  }
}

void Worker::PushToLifoSlot(TaskBase* task) {
  TaskBase* prev_lifo = std::exchange(lifo_slot_, task);
  if (prev_lifo != nullptr) {
    PushToLocalQueue(prev_lifo);
  }
}

size_t Worker::StealTasks(std::span<TaskBase*> out_buffer) {
  return local_queue_.Grab(out_buffer);
}

TaskBase* Worker::TryStealTasks(size_t series) {
  TaskBase** steal_start = &buffer_[0];
  size_t to_steal = 7;
  size_t stolen = 0;

  for (size_t t = 0; (t < series) && (to_steal > 0); ++t) {
    size_t k = twister_();
    for (size_t i = 0; (i < host_.threads_) && (to_steal > 0); ++i) {
      size_t steal_target_index = (k + i) % host_.threads_;
      if (steal_target_index == index_) {
        continue;
      }
      Worker& steal_target = host_.workers_[steal_target_index];
      size_t stolen_from_target =
          steal_target.StealTasks({steal_start, to_steal});
      if (stolen_from_target > 0) {
        steal_start += stolen_from_target;
        to_steal -= stolen_from_target;
        stolen += stolen_from_target;
      }
    }
  }
  if (stolen > 0) {
    if (stolen > 1) {
      local_queue_.PushMany({buffer_ + 1, stolen - 1});
    }
    return buffer_[0];
  } else {
    return nullptr;
  }
}
}  // namespace executor