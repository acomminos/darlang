#include "solver.h"

namespace darlang {
namespace typing {

void Typeable::Unify(Typeable& other) {
  if (parent_) {
    parent_->Unify(other);
    return;
  }
  // If we're the highest-level parent, expect a solver implementation.
  assert(solver_);
  solver_.Intersect(other.solver_);
  other.parent_ = this;
  other.solver_ = nullptr;
}

Type TypeSolver::Solve() {
}

void TypeSolver::Intersect(TypeSolver& solver) {
}

void TypeSolver::Constrain(TypeConstraint constraint) {
  constraints_.push_back(constraint);
}

Typeable* TypeSolver::Member(const std::string id) {
  if (members_.find(id) != members_.end()) {
    return members_[id];
  }
}

}  // namespace typing
}  // namespace darlang
