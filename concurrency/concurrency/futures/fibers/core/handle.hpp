#pragma once
#include <cassert>
#include <utility>

namespace futures {
class Fiber;

class FiberHandle {
  friend class Fiber;

 public:
  FiberHandle(Fiber*);
  FiberHandle();
  FiberHandle(const FiberHandle&);
  FiberHandle& operator=(const FiberHandle&);
  FiberHandle(FiberHandle&&);
  FiberHandle& operator=(FiberHandle&&);

  static FiberHandle Invalid();

  static FiberHandle CreateHandle();

  void Schedule();

  Fiber* Release();

  bool IsValid();

 private:
  Fiber* fiber_{nullptr};
};
}  // namespace futures
