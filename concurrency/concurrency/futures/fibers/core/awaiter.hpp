#pragma once
#include <cassert>
#include <concurrency/futures/fibers/core/handle.hpp>

namespace futures {
enum class AwaitEither { Continue, Suspend };

struct IAwaiter {
  virtual FiberHandle AwaitSymmSuspend(FiberHandle) = 0;
};

struct IMaybeSuspendingAwaiter : IAwaiter {
  FiberHandle AwaitSymmSuspend(FiberHandle h) override {
    auto either = AwaitMaybeSuspend(h);
    if (either == AwaitEither::Continue) {
      return h;
    } else {
      return FiberHandle::Invalid();
    }
  }

  virtual AwaitEither AwaitMaybeSuspend(FiberHandle h) = 0;
};

struct ISuspendingAwaiter : IAwaiter {
  FiberHandle AwaitSymmSuspend(FiberHandle h) override {
    AwaitSuspend(h);
    return FiberHandle::Invalid();
  }

  virtual void AwaitSuspend(FiberHandle) = 0;
};
}  // namespace futures
