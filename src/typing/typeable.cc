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
  : solver_(std::move(solver)), solve_run_({false, {}}), parent_(nullptr) {
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
    if (solve_run_.active) {
      // If we've encountered a cycle, produce a stub type that indicates this
      // is a recurrence of some parent type. This stub will be set once the
      // parent is finished generating the appropriate type.
      auto recurrence = std::make_unique<Recurrence>();
      solve_run_.recurrences.push_back(recurrence.get());
      out_type = std::move(recurrence);
      return Result::Ok();
    }

    solve_run_.active = true;
    Result res = solver_->Solve(out_type);
    assert(out_type);

    // Flag the parent type as being recursive to prevent a traversal to
    // determine infinitely-sized recursive types.
    if (solve_run_.recurrences.size() > 0) {
      out_type->set_recursive(true);
    }

    for (auto& recurrence : solve_run_.recurrences) {
      // FIXME(acomminos): should we really be overwriting the entire object?
      recurrence->set_parent_type(out_type.get());
    }

    solve_run_.active = false;
    solve_run_.recurrences = {};
    return res;
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
