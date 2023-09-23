#pragma once
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <concurrency/futures/fibers/core/fiber.hpp>

namespace futures {
void Suspend(IAwaiter* awaiter) {
  auto& current_fiber = Fiber::CurrentFiber();
  current_fiber.Suspend(awaiter);
}
}  // namespace futures
