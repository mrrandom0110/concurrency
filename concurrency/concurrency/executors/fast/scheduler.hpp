#pragma once
#include <concurrency/executors/fast/hint.hpp>
#include <concurrency/executors/fast/queues/global_queue.hpp>
#include <concurrency/executors/fast/task.hpp>
#include <concurrency/executors/fast/worker.hpp>
#include <cstdlib>
#include <deque>

namespace executor {
class Worker;

class Scheduler {
  friend class Worker;

 public:
  Scheduler(std::size_t);

  ~Scheduler();

  void Submit(TaskBase*, SchedulerHint);

  void Start();

  void Stop();

 private:
  GlobalQueue global_queue_;
  std::size_t threads_;
  std::deque<Worker> workers_;
};
}  // namespace executor