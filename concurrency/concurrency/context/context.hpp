#pragma once
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

#include <concurrency/context/trampoline.hpp>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mem_view.hpp"
#include "mmap_allocation.hpp"

namespace context {

struct MachineContext {
  void* rsp_;

  void Setup(MutableMemView, context::ITrampoline*);

  void SwitchTo(MachineContext&);
};
}  // namespace context
