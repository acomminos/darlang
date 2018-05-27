#include "typing/function_solver.h"

namespace darlang {
namespace typing {

FunctionSolver::FunctionSolver(int num_args) : args_(num_args), yield_(Typeable::Create()) {
  for (auto& arg_typeable : args_) {
    arg_typeable = Typeable::Create();
  }
}

Result FunctionSolver::MergeInto(FunctionSolver& other) {
  if (other.num_args() != num_args()) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "conflicting argument count");
  }

  for (int i = 0; i < args_.size(); i++) {
    auto& self_arg = args_[i];
    auto& other_arg = other.args()[i];

    Result arg_result = other_arg->Unify(self_arg);
    if (!arg_result) {
      return arg_result;
    }
  }

  Result yield_result = other.yield()->Unify(yield_);
  if (!yield_result) {
    return yield_result;
  }

  return Result::Ok();
}

Result FunctionSolver::Solve(std::unique_ptr<Type>& out_type) {
  std::vector<std::unique_ptr<Type>> arg_types(args_.size());
  for (int i = 0; i < args_.size(); i++) {
    auto arg_result = args_[i]->Solve(arg_types[i]);
    if (!arg_result) {
      // TODO(acomminos): nest result
      return arg_result;
    }
  }

  std::unique_ptr<Type> yield_type;
  auto yield_result = yield_->Solve(yield_type);
  if (!yield_result) {
    // TODO(acomminos): nest result
    return yield_result;
  }

  out_type = std::make_unique<Function>(std::move(arg_types), std::move(yield_type));
  return Result::Ok();
}

}  // namespace typing
}  // namespace darlang
