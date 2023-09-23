#pragma once
#include <cassert>
#include <cstdlib>

namespace context {
class MutableMemView {
 public:
  MutableMemView();

  MutableMemView(const MutableMemView&);

  MutableMemView& operator=(const MutableMemView&);

  MutableMemView(char*, size_t);

  char* Start();

  char* End();

  size_t Size();

 private:
  char* start_;
  size_t size_;
};
}  // namespace context