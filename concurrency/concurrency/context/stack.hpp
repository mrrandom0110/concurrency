#pragma once
#include "mem_view.hpp"
#include "mmap_allocation.hpp"

namespace context {
class Stack {
 public:
  static Stack AllocateBytes(size_t);
  static Stack AllocatePages(size_t);

  MutableMemView MutView();

 private:
  Stack(MmapAllocation);

 private:
  MmapAllocation allocation_;
};
}  // namespace context