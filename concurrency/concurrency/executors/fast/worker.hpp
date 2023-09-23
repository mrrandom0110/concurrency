#pragma once
#include <concurrency/utility/intrusive/intrusive_node.hpp>
#include <cstdlib>
#include <deque>
#include <optional>
#include <random>
#include <span>
#include <thread>
#include <utility>

#include "hint.hpp"
#include "queues/ring_buffer.hpp"
#include "scheduler.hpp"
#include "task.hpp"

namespace executor {
class Scheduler;

class Worker : utility::IntrusiveNode<Worker> {
  static const size_t kLocalQueueCapacity = 256;
  static const size_t kGlobalPoll = 41;
  static thread_local Worker* current;

  friend class Scheduler;

 public:
  Worker(Scheduler&, size_t);

  void Start();

  void Join();

  static Worker& CurrentWorker() { return *current; }

 private:
  // Run loop
  void Work();

  TaskBase* TryPickTask();

  void PushToLifoSlot(TaskBase*);

  void PushToLocalQueue(TaskBase*);

  void OffloadTasksToGlobalQueue(TaskBase*);

  void LocalPush(TaskBase*, SchedulerHint);

  TaskBase* GrabTasksFromGlobalQueue();

  TaskBase* TryPickTaskFromGlobalQueue();

  TaskBase* TryPickTaskFromLifoSlot();

  TaskBase* TryStealTasks(size_t);

  size_t StealTasks(std::span<TaskBase*>);

 private:
  // Worker thread
  std::mt19937_64 twister_;
  std::optional<std::thread> thread_;
  Scheduler& host_;
  TaskBase* lifo_slot_{nullptr};
  size_t index_{0};
  size_t lifo_steps_{0};
  size_t iter_{0};
  TaskBase* buffer_[kLocalQueueCapacity];
  WorkStealingQueue<kLocalQueueCapacity, TaskBase> local_queue_;
};
}  // namespace executor
