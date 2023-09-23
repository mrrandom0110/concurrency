#pragma once
#include <atomic>
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <concurrency/futures/fibers/core/handle.hpp>
#include <concurrency/futures/fibers/sched/suspend.hpp>

namespace futures {
class Mutex {
  using State = uintptr_t;

  enum States : uintptr_t {
    Unlocked = 0,
    LockedNoWaiters = 1,
    // Waiter*
  };

  struct WaitNode {
    FiberHandle fiber;
    WaitNode* next;
  };

  enum class LockStatus { Acquired, Parked };

  class LockAwaiter : public IMaybeSuspendingAwaiter {
   public:
    explicit LockAwaiter(Mutex* mutex) : mutex_(mutex) {}

    AwaitEither AwaitMaybeSuspend(FiberHandle h) override {
      waiter_.fiber = h;
      auto status = mutex_->AcquireOrPark(&waiter_);
      if (status == LockStatus::Acquired) {
        return AwaitEither::Continue;
      } else {
        return AwaitEither::Suspend;
      }
    }

   private:
    Mutex* mutex_;
    WaitNode waiter_;
  };

  class UnlockAwaiter : public IAwaiter {
   public:
    explicit UnlockAwaiter(FiberHandle next_owner) : next_owner_(next_owner) {}

    FiberHandle AwaitSymmSuspend(FiberHandle curr_owner) override {
      FiberHandle next_owner = next_owner_;
      curr_owner.Schedule();
      // ~ UnlockAwaiter
      return next_owner;
    }

   private:
    FiberHandle next_owner_;
  };

 public:
  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

  void Lock() {  // NOLINT
    if (TryLock()) {
      return;  // optimization, fast path
    }
    LockAwaiter awaiter{this};
    Suspend(&awaiter);
  }

  void Unlock() {  // NOLINT
    if (waiters_head_ != nullptr) {
      Resume(TakeNextOwner());
    } else {
      while (true) {
        State curr = state_.load();
        if (curr == States::LockedNoWaiters) {
          if (CasState(States::LockedNoWaiters, States::Unlocked)) {
            return;
          }
        } else {
          WaitNode* waiters =
              (WaitNode*)state_.exchange(States::LockedNoWaiters);
          waiters_head_ = Reverse(waiters);
          Resume(TakeNextOwner());
          return;
        }
      }
    }
  }

  bool TryLock() { return CasState(States::Unlocked, States::LockedNoWaiters); }

 private:
  bool CasState(State from, State to,
                std::memory_order mo = std::memory_order_seq_cst) {
    return state_.compare_exchange_strong(from, to, mo);
  }

  LockStatus AcquireOrPark(WaitNode* waiter) {
    while (true) {
      State curr = state_.load();
      if (curr == States::Unlocked) {
        if (TryLock()) {
          return LockStatus::Acquired;
        }
      } else {
        if (curr == States::LockedNoWaiters) {
          waiter->next = nullptr;
        } else {
          waiter->next = (WaitNode*)curr;
        }

        if (state_.compare_exchange_weak(curr, (State)waiter)) {
          return LockStatus::Parked;
        }
      }
    }
  }

  static WaitNode* Reverse(WaitNode* head) {
    WaitNode* prev = nullptr;
    WaitNode* curr = head;
    while (curr != nullptr) {
      WaitNode* next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
    return prev;
  }

  FiberHandle TakeNextOwner() {
    FiberHandle next_owner = waiters_head_->fiber;
    waiters_head_ = waiters_head_->next;
    return next_owner;
  }

  void Resume(FiberHandle next_owner) {
    UnlockAwaiter awaiter{next_owner};
    Suspend(&awaiter);
  }

 private:
  // Flag + Waiters (Stack, Push)
  std::atomic<State> state_{States::Unlocked};
  WaitNode* waiters_head_{nullptr};
};
}  // namespace futures
