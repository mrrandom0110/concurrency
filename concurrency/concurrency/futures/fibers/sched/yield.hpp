#pragma once
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <concurrency/futures/fibers/sched/suspend.hpp>

namespace futures {
struct YieldAwaiter : ISuspendingAwaiter {
  void AwaitSuspend(FiberHandle fiber) override { fiber.Schedule(); }
};

void Yield() {
  YieldAwaiter awaiter;
  Suspend(&awaiter);
}
}  // namespace futures
