#pragma once
#include <cassert>

namespace utility {
template <typename T>
class IntrusiveList {
 public:
  IntrusiveList() = default;
  IntrusiveList(const IntrusiveList&) = delete;
  IntrusiveList& operator=(const IntrusiveList&) = delete;
  IntrusiveList(IntrusiveList&&) = delete;
  IntrusiveList& operator=(IntrusiveList&&) = delete;

  void Push(T* node) {
    assert(node != nullptr);
    if (head_ == nullptr) {
      head_ = tail_ = node;
    } else {
      tail_->next = node;
      tail_ = node;
    }
  }

  void Append(IntrusiveList& other) noexcept {
    if (other.IsEmpty()) {
      return;
    }
    if (IsEmpty()) {
      head_ = other.head_;
      tail_ = other.tail_;
      size_ += other.size_;
    } else {
      tail_->next = other.head_;
      tail_ = other.tail_;
      size_ += other.size_;
    }
    other.Reset();
  }

  T* Pop() noexcept {
    if (IsEmpty()) {
      return nullptr;
    }
    --size_;
    T* item = head_;
    if (head_ == tail_) {
      head_ = tail_ = nullptr;
    } else {
      head_ = head_->next;
    }
    item->next = nullptr;
    return item;
  }

  void Reset() { head_ = tail_ = nullptr; }

  bool IsEmpty() const noexcept { return head_ == nullptr; }

  std::size_t Size() const noexcept { return size_; }

 private:
  T* head_{nullptr};
  T* tail_{nullptr};
  std::size_t size_;
};
}