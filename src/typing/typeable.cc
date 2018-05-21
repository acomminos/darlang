#include "typing/typeable.h"
#include "typing/solver.h"

namespace darlang {
namespace typing {

Typeable::Typeable() : parent_(nullptr) {
  solver_ = std::make_unique<TypeSolver>();
}

Result Typeable::Unify(Typeable& other) {
  // Traverse to the roots of each of the typeables being merged.
  if (parent_) {
    return parent_->Unify(other);
  }
  if (other.parent_) {
    return Unify(*(other.parent_));
  }

  // If we're the highest-level parents, expect a solver implementation.
  assert(solver_);
  assert(other.solver_);

  // If the typeables are part of the same component, they've already been
  // unified.
  if (solver_ == other.solver_) {
    return Result::Ok();
  }

  auto result = solver_->Unify(*other.solver_);
  if (result) {
    other.parent_ = this;
    other.solver_ = nullptr;
  }
  return result;
}

TypeSolver& Typeable::Solver() {
  if (parent_) {
    return parent_->Solver();
  }
  return *solver_;
}

}  // namespace typing
}  // namespace darlang
