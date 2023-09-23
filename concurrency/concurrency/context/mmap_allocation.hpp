#pragma once
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <concurrency/context/trampoline.hpp>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "mem_view.hpp"
#include "page.hpp"

namespace context {
class MmapAllocation {
 public:
  MmapAllocation();

  MmapAllocation(MmapAllocation&&);

  MmapAllocation& operator=(MmapAllocation&&);

  MmapAllocation(const MmapAllocation&) = delete;

  MmapAllocation& operator=(const MmapAllocation&) = delete;

  ~MmapAllocation();

  static MmapAllocation AllocateBytes(size_t);

  static MmapAllocation AllocatePages(size_t);

  MutableMemView MutView();

  void ProtectPages(size_t, size_t);

  static size_t PagesToBytes(size_t);

  size_t PageSize();

  char* Start();

  char* End();

  size_t Size();

  void Deallocate();

 private:
  MmapAllocation(char*, size_t);

  void Reset();

 private:
  char* start_;
  size_t size_;
};
}  // namespace context
