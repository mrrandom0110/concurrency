#include "stack.hpp"

namespace context {
Stack Stack::AllocateBytes(size_t bytes) {
  auto allocation = MmapAllocation::AllocateBytes(bytes);
  allocation.ProtectPages(0, 1);
  return Stack{std::move(allocation)};
}

Stack Stack::AllocatePages(size_t count) {
  auto allocation = MmapAllocation::AllocatePages(count + 1);
  // allocation.ProtectPages(0, 1);
  return Stack{std::move(allocation)};
}

MutableMemView Stack::MutView() { return allocation_.MutView(); }

Stack::Stack(MmapAllocation allocation) : allocation_(std::move(allocation)) {}

}  // namespace context
