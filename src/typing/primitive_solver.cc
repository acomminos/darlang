#include "typing/primitive_solver.h"

namespace darlang {
namespace typing {

PrimitiveSolver::PrimitiveSolver(PrimitiveType primitive)
  : primitive_(primitive) {
}

Result PrimitiveSolver::MergeInto(PrimitiveSolver& other) {
  if (other.primitive() != primitive_) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "cannot unify incompatible primitives");
  }

  return Result::Ok();
}

Result PrimitiveSolver::Solve(std::unique_ptr<Type>& out_type) {
  out_type = std::make_unique<Primitive>(primitive_);
  return Result::Ok();
}

}  // namespace typing
}  // namespace darlang
