#pragma once
#include <atomic>
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <concurrency/futures/fibers/core/handle.hpp>
#include <concurrency/futures/fibers/sched/suspend.hpp>

namespace futures {

class Event {
  using State = uintptr_t;

  enum States : uintptr_t { Init, FireStatus };

  enum WaitStatus { Park, Continue };

  struct WaitNode {
    WaitNode* next;
    FiberHandle fiber;
  };

  class WaitAwaiter : public IMaybeSuspendingAwaiter {
   public:
    explicit WaitAwaiter(Event* event) : event_(event) {}

    AwaitEither AwaitMaybeSuspend(FiberHandle fiber) override {
      node_.fiber = fiber;
      auto status = event_->TryPark(&node_);
      if (status == WaitStatus::Park) {
        return AwaitEither::Suspend;
      }
      return AwaitEither::Continue;
    }

   private:
    Event* event_;
    WaitNode node_;
  };

 public:
  void Wait() {
    WaitAwaiter awaiter{this};
    Suspend(&awaiter);
  }

  void Fire() { SetEvent(); }

 private:
  WaitStatus TryPark(WaitNode* waiter) {
    while (true) {
      State state = state_.load();
      if (state == States::Init) {
        waiter->next = nullptr;
        if (state_.compare_exchange_weak(state, (State)waiter)) {
          return WaitStatus::Park;
        }
      } else if (state != States::FireStatus) {
        waiter->next = (WaitNode*)state;
        if (state_.compare_exchange_weak(state, (State)waiter)) {
          return WaitStatus::Park;
        }
      } else {
        return WaitStatus::Continue;
      }
    }
  }

  void SetEvent() {
    WaitNode* node = nullptr;
    while (true) {
      State state = state_.load();
      if (CasState(state, States::FireStatus)) {
        if (state != States::Init) {
          node = (WaitNode*)state;
        }
        break;
      }
    }
    while (node != nullptr) {
      auto* next = node->next;
      node->fiber.Schedule();
      node = next;
    }
  }

  bool CasState(State from, State to) {
    return state_.compare_exchange_strong(from, to);
  }

 private:
  std::atomic<State> state_{States::Init};
};
};  // namespace futures
