#include "coro.hpp"

namespace runtime {

thread_local Coroutine* Coroutine::current = nullptr;

Coroutine::Coroutine(Routine routine)
    : routine_(std::move(routine)),
      stack_(::context::Stack::AllocatePages(kDefaultStackPages)) {
  SetupContext();
}

void Coroutine::Resume() {
  scheduler_ = current;
  current = this;
  caller_context_.SwitchTo(coro_context_);
  current = scheduler_;
  if (exception_ptr_) {
    std::rethrow_exception(exception_ptr_);
  }
}

void Coroutine::Suspend() {
  assert(current);
  current->coro_context_.SwitchTo(current->caller_context_);
}

void Coroutine::Run() noexcept {
  try {
    routine_();
  } catch (...) {
    exception_ptr_ = std::current_exception();
  }
  completed_ = true;
  coro_context_.ExitTo(caller_context_);
}

bool Coroutine::IsCompleted() const { return completed_; }

void Coroutine::SetupContext() { coro_context_.Setup(stack_.MutView(), this); }
}  // namespace runtime
