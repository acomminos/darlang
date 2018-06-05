#include "typing/typeable.h"
#include "typing/types.h"
#include "typing/solver.h"

namespace darlang {
namespace typing {

/* static */
TypeablePtr Typeable::Create(std::unique_ptr<Solver> solver) {
  return std::make_shared<Typeable>(std::move(solver));
}

Typeable::Typeable(std::unique_ptr<Solver> solver)
  : solver_(std::move(solver)), parent_(nullptr) {
}

Result Typeable::Unify(const TypeablePtr& other) {
  // Traverse to the roots of each of the typeables being merged.
  if (parent_) {
    return parent_->Unify(other);
  }
  if (other->parent_) {
    return Unify(other->parent_);
  }

  // If the typeables share a common root, they are already unified.
  if (shared_from_this() == other) {
    return Result::Ok();
  }

  // If both typeables have bound solvers, merge the solvers.
  if (solver_ && other->solver_) {
    auto result = solver_->Merge(*other->solver_);
    if (result) {
      other->parent_ = shared_from_this();
      other->solver_ = nullptr;
    }
    return result;
  } else if (solver_) {
    other->parent_ = shared_from_this();
    return Result::Ok();
  } else if (other->solver_) {
    parent_ = other;
    return Result::Ok();
  }

  // If neither implementation had a solver, anchor the other typeable to this.
  other->parent_ = shared_from_this();
  return Result::Ok();
}

Result Typeable::Solve(std::unique_ptr<Type>& out_type) {
  if (parent_) {
    return parent_->Solve(out_type);
  }
  if (solver_) {
    return solver_->Solve(out_type);
  }
  // If we reached the root of the union-find tree and there is no solver, the
  // typeable lacks a specialization.
  return Result::Error(ErrorCode::TYPE_INDETERMINATE, "no specialization constrained");
}

std::unique_ptr<Type> Typeable::Solve() {
  std::unique_ptr<Type> type;
  assert(Solve(type));
  return std::move(type);
}

bool Typeable::IsSolvable() {
  std::unique_ptr<Type> stub;
  return Solve(stub);
}

}  // namespace typing
}  // namespace darlang
