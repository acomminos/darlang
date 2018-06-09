#ifndef DARLANG_SRC_TYPING_DISJOINT_SOLVER_H_
#define DARLANG_SRC_TYPING_DISJOINT_SOLVER_H_

#include "typing/solver.h"

namespace darlang {
namespace typing {

// A disjoint solver solves for a type that is of exactly one of a set of
// disjoint typeables.
class DisjointSolver : public Solver {
 public:
  Result Merge(Solver& other) override { return other.MergeInto(*this); }
  Result MergeInto(DisjointSolver& other) override;
  Result Solve(std::unique_ptr<Type>& out_type) override;

  // Adds the given typeable to end of the disjoint solver.
  Result Add(TypeablePtr typeable);

  const std::vector<TypeablePtr>& types() const { return types_; }

 private:
  std::vector<TypeablePtr> types_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_DISJOINT_SOLVER_H_
