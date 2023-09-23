#include <concurrency/futures/fibers/core/fiber.hpp>
#include <concurrency/futures/fibers/core/handle.hpp>

namespace futures {
FiberHandle::FiberHandle(Fiber* fiber) : fiber_(fiber) {}

FiberHandle::FiberHandle() : fiber_{nullptr} {}

FiberHandle::FiberHandle(const FiberHandle& other) { fiber_ = other.fiber_; }

FiberHandle& FiberHandle::operator=(const FiberHandle& other) {
  fiber_ = other.fiber_;
  return *this;
}

FiberHandle::FiberHandle(FiberHandle&& other) {
  fiber_ = other.fiber_;
  other.fiber_ = nullptr;
}

FiberHandle& FiberHandle::operator=(FiberHandle&& other) {
  fiber_ = other.fiber_;
  other.fiber_ = nullptr;
  return *this;
}

FiberHandle FiberHandle::Invalid() { return FiberHandle{nullptr}; }

FiberHandle FiberHandle::CreateHandle() {
  Fiber& fiber = Fiber::CurrentFiber();
  return FiberHandle{&fiber};
}

void FiberHandle::Schedule() { fiber_->Schedule(); }
Fiber* FiberHandle::Release() {
  assert(IsValid());
  return std::exchange(fiber_, nullptr);
}

bool FiberHandle::IsValid() { return fiber_ != nullptr; }
}  // namespace futures
