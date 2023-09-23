#include <concurrency/futures/fibers/core/fiber.hpp>
#include <iostream>

namespace futures {
thread_local Fiber* Fiber::current = nullptr;

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(scheduler), coro_impl_(std::move(routine)) {}

Fiber& Fiber::CurrentFiber() { return *current; }

void Fiber::Schedule() { scheduler_.Submit(this, SchedulerHint::Default); }

void Fiber::Suspend(Awaiter* awaiter) {
  awaiter_ = awaiter;
  coro_impl_.Suspend();
}

void Fiber::Run() noexcept { RunChain(this); }

void Fiber::RunChain(Fiber* from) {
  Fiber* next = from;
  while (next != nullptr) {
    next = next->Step();
  }
}

Fiber* Fiber::Step() {
  current = this;
  coro_impl_.Resume();
  current = nullptr;
  Awaiter* awaiter = std::exchange(awaiter_, nullptr);
  if (awaiter != nullptr) {
    auto next = awaiter->AwaitSymmSuspend(FiberHandle{this});
    if (next.IsValid()) {
      return next.Release();
    } else {
      return nullptr;
    }
  }
  delete this;
  return nullptr;  // for compiler
}

}  // namespace futures
