#include <concurrency/executors/fast/scheduler.hpp>
#include <concurrency/futures/fibers/core/fiber.hpp>
#include <concurrency/futures/fibers/sched/yield.hpp>
#include <concurrency/futures/fibers/sync/mutex.hpp>
#include <expected>
#include <iostream>
#include <span>
#include <stdexcept>

void foo() { foo(); }

int main() {
  executor::Scheduler scheduler{4};
  std::size_t j = 0;
  std::size_t k = 0;
  futures::Fiber* fiber = new futures::Fiber(scheduler, [&] {
    while (k < 1000) {
      auto* f = new futures::Fiber(scheduler, [] { std::cout << "Hello\n"; });
      f->Schedule();
      ++k;
    }
  });
  scheduler.Submit(fiber, SchedulerHint::External);
  scheduler.Start();
  return 0;
}
