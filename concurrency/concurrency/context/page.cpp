#include "page.hpp"

#include <iostream>

namespace context {
PageSizeDetector::PageSizeDetector() : page_size_(sysconf(_SC_PAGESIZE)) {}

std::size_t PageSizeDetector::PageSize() const { return page_size_; }
}  // namespace context
