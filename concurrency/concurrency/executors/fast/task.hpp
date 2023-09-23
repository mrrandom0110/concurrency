#pragma once
#include <concurrency/utility/intrusive/intrusive_node.hpp>

namespace executor {
struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
};

struct TaskBase : ITask, utility::IntrusiveNode<TaskBase> {};

struct TaskFunction : ITask, utility::IntrusiveNode<TaskFunction> {};

}  // namespace executor