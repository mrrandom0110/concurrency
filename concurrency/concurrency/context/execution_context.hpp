#pragma once
#include "context.hpp"
#include "trampoline.hpp"

namespace context {
class ExecutionContext : public ::context::ITrampoline {
 public:
  ExecutionContext();
  void Setup(MutableMemView, context::ITrampoline*);

  void SwitchTo(ExecutionContext&);

  void ExitTo(ExecutionContext&);

 private:
  void Run() noexcept override;

 private:
  ITrampoline* user_trampoline_{nullptr};
  MachineContext machine_;
};
}  // namespace context