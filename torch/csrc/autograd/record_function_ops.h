#pragma once
#include <torch/csrc/autograd/record_function.h>
#include <torch/csrc/utils/future.h>

namespace torch {
namespace autograd {
namespace profiler {
// Creates a new profiling scope using RecordFunction and invokes its starting
// callbacks.
c10::intrusive_ptr<RecordFunction> record_function_enter(
    const std::string& name);

// Cast Tensor that was created with at::cpp_custom_type_hack back to
// RecordFunction. This is a temporary workaround until RecordFunction is
// registered as a custom C++ class.
TORCH_API RecordFunction& getRecordFunctionFromTensor(const at::Tensor& handle);

// Schedules RecordFunction's end callbacks to be run on completion of a future.
template <typename T>
TORCH_API void _call_end_callbacks_on_fut(
    const at::Tensor& handle,
    const std::shared_ptr<torch::utils::Future<T>> fut) {
  // Add a callback onto the future to mark run RecordFunction's end callbacks
  // when the future is completed.
  fut->addCallback(
      // Copy handle by value to persist after the python context manager is
      // exited.
      [handle](
          const T& /* unused */,
          const c10::optional<torch::utils::FutureError>& /* unused */) {
        TORCH_INTERNAL_ASSERT(
            handle.defined(),
            "Undefined RecordFunction handle. This can happen if the handle is "
            "not correctly persisted and is destroyed before the future is "
            "realized.");
        auto& rec = getRecordFunctionFromTensor(handle);
        rec._end();
      });
}

// Ends the profiling scope created with record_function_enter.
void record_function_exit(
    const c10::intrusive_ptr<RecordFunction>& instance);

} // namespace profiler
} // namespace autograd
} // namespace torch