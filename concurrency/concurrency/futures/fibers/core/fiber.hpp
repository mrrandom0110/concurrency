#pragma once
#include <concurrency/context/trampoline.hpp>
#include <concurrency/coroutines/coro.hpp>
#include <concurrency/executors/fast/hint.hpp>
#include <concurrency/executors/fast/scheduler.hpp>
#include <concurrency/executors/fast/task.hpp>
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <function2/function2.hpp>
#include <string>

namespace futures {
class Fiber : public executor::TaskBase, context::ITrampoline {
  using Scheduler = executor::Scheduler;
  using Awaiter = IAwaiter;
  using Routine = fu2::unique_function<void()>;

  static thread_local Fiber* current;

 public:
  Fiber(Scheduler&, Routine);

  void Suspend(Awaiter*);

  void Schedule();

  void Run() noexcept override;

  Fiber* Step();

  void RunChain(Fiber* from);

  Scheduler& CurrentScheduler();

  static Fiber& CurrentFiber();

 private:
  runtime::Coroutine coro_impl_;
  Scheduler& scheduler_;
  Awaiter* awaiter_{nullptr};
};
}  // namespace futures
