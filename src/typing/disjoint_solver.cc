#include "typing/disjoint_solver.h"

namespace darlang {
namespace typing {

Result DisjointSolver::MergeInto(DisjointSolver& other) {
  const auto& self_types = types();
  const auto& other_types = other.types();
  if (self_types.size() != other_types.size()) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "conflicting disjoint type count");
  }

  for (int i = 0; i < self_types.size(); i++) {
    Result res;
    if (!(res = other_types[i]->Unify(self_types[i]))) {
      return res;
    }
  }

  return Result::Ok();
}

Result DisjointSolver::Solve(std::unique_ptr<Type>& out_type) {
  if (types_.size() == 0) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "disjoint solver has no subtypes");
  }

  std::vector<std::unique_ptr<Type>> materialized(types_.size());
  for (int i = 0; i < types_.size(); i++) {
    Result res;
    if (!(res = types_[i]->Solve(materialized[i]))) {
      return res;
    }
  }
  out_type = std::make_unique<DisjointUnion>(std::move(materialized));
  return Result::Ok();
}

Result DisjointSolver::Add(TypeablePtr typeable) {
  types_.push_back(typeable);
  return Result::Ok();
}

}  // namespace typing
}  // namespace darlang
