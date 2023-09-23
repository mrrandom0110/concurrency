#pragma once
#include <atomic>
#include <concurrency/futures/fibers/core/awaiter.hpp>
#include <concurrency/futures/fibers/core/handle.hpp>
#include <concurrency/futures/fibers/sched/suspend.hpp>

namespace futures {
class WaitGroup {
  using State = uintptr_t;

  struct WaitNode {
    FiberHandle fiber;
    WaitNode* next;
  };

  enum States : uintptr_t { Init, LastDone };

  enum ParkStatus { Park, Continue };

  class GroupWait : public IMaybeSuspendingAwaiter {
   public:
    explicit GroupWait(WaitGroup* wg) : wg_(wg) {}

    AwaitEither AwaitMaybeSuspend(FiberHandle fiber) override {
      node_.fiber = fiber;
      auto status = wg_->ParkOrContinue(&node_);
      if (status == ParkStatus::Park) {
        return AwaitEither::Suspend;
      }
      return AwaitEither::Continue;
    }

   private:
    WaitGroup* wg_;
    WaitNode node_;
  };

 public:
  void Add(size_t count) { count_.fetch_add(count); }

  void Wait() {
    GroupWait awaiter{this};
    Suspend(&awaiter);
  }

  void Done() {
    auto count = count_.fetch_sub(1);
    if (count <= 1) {
      ResumeAll();
    }
  }

 private:
  void ResumeAll() {
    WaitNode* node = nullptr;
    while (true) {
      auto state = state_.load();
      if (CasState(state, States::LastDone)) {
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

  ParkStatus ParkOrContinue(WaitNode* node) {
    while (true) {
      auto state = state_.load();
      if (state == States::Init) {
        node->next = nullptr;
        if (state_.compare_exchange_weak(state, (State)node)) {
          return ParkStatus::Park;
        }
      } else if (state != States::LastDone) {
        node->next = (WaitNode*)state;
        if (state_.compare_exchange_weak(state, (State)node)) {
          return ParkStatus::Park;
        }
      } else {
        return ParkStatus::Continue;
      }
    }
  }

  bool CasState(State from, State to) {
    return state_.compare_exchange_strong(from, to);
  }

 private:
  std::atomic<uint32_t> count_{0};
  std::atomic<WaitNode*> head_{nullptr};
  std::atomic<State> state_{States::Init};
};

}  // namespace futures
