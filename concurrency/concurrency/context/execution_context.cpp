#include "execution_context.hpp"

namespace context {

ExecutionContext::ExecutionContext() = default;
void ExecutionContext::Setup(MutableMemView stack,
                             ::context::ITrampoline* trampoline) {
  user_trampoline_ = trampoline;
  machine_.Setup(stack, this);
}

void ExecutionContext::SwitchTo(ExecutionContext& target_context) {
  machine_.SwitchTo(target_context.machine_);
}

void ExecutionContext::ExitTo(ExecutionContext& target_context) {
  machine_.SwitchTo(target_context.machine_);
}

void ExecutionContext::Run() noexcept { user_trampoline_->Run(); }

ITrampoline* user_trampoline_{nullptr};
MachineContext machine_;
}  // namespace context
