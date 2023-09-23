#include "mmap_allocation.hpp"

namespace context {

static PageSizeDetector page_size_detector;

MmapAllocation::MmapAllocation() { Reset(); }

MmapAllocation::MmapAllocation(MmapAllocation&& rhs) {
  start_ = rhs.start_;
  size_ = rhs.size_;
  rhs.Reset();
}

MmapAllocation& MmapAllocation::operator=(MmapAllocation&& rhs) {
  Deallocate();
  start_ = rhs.start_;
  size_ = rhs.size_;
  rhs.Reset();
  return *this;
}

MmapAllocation::~MmapAllocation() { Deallocate(); }

MmapAllocation MmapAllocation::AllocateBytes(size_t bytes) {
  char* start = new char[bytes];
  return MmapAllocation{(char*)start, bytes};
}

MmapAllocation MmapAllocation::AllocatePages(size_t count) {
  size_t size = MmapAllocation::PagesToBytes(count);
  char* start = new char[size];
  return MmapAllocation{static_cast<char*>(start), size};
}

MutableMemView MmapAllocation::MutView() { return {start_, size_}; }

void MmapAllocation::ProtectPages(size_t start_index, size_t count) {
  int ret = mprotect(
      /*addr=*/(void*)(start_ + MmapAllocation::PagesToBytes(start_index)),
      /*len=*/MmapAllocation::PagesToBytes(count),
      /*prot=*/PROT_NONE);
  if (ret == -1) {
    //   std::cerr << strerror(errno);
    std::string s = "Cannot protect pages" + std::string(strerror(errno));
    throw std::runtime_error(s.c_str());
  }
}

size_t MmapAllocation::PagesToBytes(size_t count) {
  return page_size_detector.PageSize() * count;
}

char* MmapAllocation::Start() { return start_; }

char* MmapAllocation::End() { return start_ + size_; }

size_t MmapAllocation::Size() { return size_; }

void MmapAllocation::Deallocate() {
  if (start_ == nullptr) {
    return;
  }
  delete[] start_;
}

MmapAllocation::MmapAllocation(char* start, size_t size)
    : start_(start), size_(size) {}

void MmapAllocation::Reset() {
  start_ = nullptr;
  size_ = 0;
}
}  // namespace context