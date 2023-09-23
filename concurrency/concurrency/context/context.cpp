#include "context.hpp"

extern "C" void* SetupMachineContext(void* stack, void* trampoline, void* arg);

extern "C" void SwitchMachineContext(void* from_rsp, void* to_rsp);

namespace context {
void MachineContextTrampoline(void*, void*, void*, void*, void*, void*, void*,
                              void*, void* arg9) {
  ::context::ITrampoline* t = (::context::ITrampoline*)arg9;
  t->Run();
}
void MachineContext::Setup(MutableMemView stack,
                           ::context::ITrampoline* trampoline) {
  rsp_ = SetupMachineContext(static_cast<void*>(stack.End()),
                             (void*)MachineContextTrampoline, trampoline);
}

void MachineContext::SwitchTo(MachineContext& target_context) {
  SwitchMachineContext(&rsp_, &target_context.rsp_);
}
}  // namespace context