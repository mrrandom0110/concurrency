#pragma once
#include <unistd.h>

#include <cstdlib>

namespace context {
class PageSizeDetector {
 public:
  PageSizeDetector();

  std::size_t PageSize() const;

 private:
  std::size_t page_size_;
};
}  // namespace context
