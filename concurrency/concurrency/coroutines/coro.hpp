#pragma once
#include <concurrency/context/context.hpp>
#include <concurrency/context/execution_context.hpp>
#include <concurrency/context/stack.hpp>
#include <concurrency/context/trampoline.hpp>
#include <function2/function2.hpp>

namespace runtime {

class Coroutine;

class Coroutine : private context::ITrampoline {
  using Routine = fu2::unique_function<void()>;

  static thread_local Coroutine* current;
#if CORO_STACK == 1
  static const size_t kDefaultStackPages = 4;
#else
  static const size_t kDefaultStackPages = 16;
#endif

 public:
  Coroutine(Routine);

  void Resume();

  static void Suspend();

  void Run() noexcept override;

  bool IsCompleted() const;

 private:
  void SetupContext();

 private:
  Routine routine_;
  std::exception_ptr exception_ptr_;
  ::context::Stack stack_;
  ::context::ExecutionContext coro_context_;
  ::context::ExecutionContext caller_context_;
  bool completed_;
  Coroutine* scheduler_;
};
}  // namespace runtime
