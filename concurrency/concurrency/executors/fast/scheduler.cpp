#include "scheduler.hpp"

namespace executor {
Scheduler::Scheduler(std::size_t threads) : threads_(threads) {}

Scheduler::~Scheduler() { Stop(); }

void Scheduler::Submit(TaskBase* task,
                       SchedulerHint hint = SchedulerHint::Default) {
  if (hint == SchedulerHint::External) {
    global_queue_.PushOne(task);
  } else {
    Worker& worker = Worker::CurrentWorker();
    worker.LocalPush(task, hint);
  }
}

void Scheduler::Start() {
  for (std::size_t i = 0; i < threads_; ++i) {
    workers_.emplace_back(*this, i);
  }
  for (auto& worker : workers_) {
    worker.Start();
  }
}

void Scheduler::Stop() {
  for (auto& worker : workers_) {
    worker.Join();
  }
}
}  // namespace executor
