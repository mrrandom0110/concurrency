#include "mem_view.hpp"

#include <cassert>
#include <cstdlib>

namespace context {

MutableMemView::MutableMemView() : start_(nullptr), size_(0) {}

MutableMemView::MutableMemView(const MutableMemView& mem_view) {
  start_ = mem_view.start_;
  size_ = mem_view.size_;
}

MutableMemView& MutableMemView::operator=(const MutableMemView& mem_view) {
  start_ = mem_view.start_;
  size_ = mem_view.size_;
  return *this;
}

MutableMemView::MutableMemView(char* start, size_t size)
    : start_(start), size_(size) {}

char* MutableMemView::Start() { return start_; }

char* MutableMemView::End() {
  assert(start_ != nullptr);
  return start_ + size_;
}

size_t MutableMemView::Size() { return size_; }
}  // namespace context
