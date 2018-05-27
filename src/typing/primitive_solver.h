#ifndef DARLANG_SRC_TYPING_PRIMITIVE_SOLVER_H_
#define DARLANG_SRC_TYPING_PRIMITIVE_SOLVER_H_

#include "typing/solver.h"
#include "typing/types.h"

namespace darlang {
namespace typing {

class PrimitiveSolver : public Solver {
 public:
  PrimitiveSolver(PrimitiveType primitive);

  Result Merge(Solver& solver) override { return solver.MergeInto(*this); }
  Result MergeInto(PrimitiveSolver& other) override;
  Result Solve(std::unique_ptr<Type>& out_type) override;

  PrimitiveType primitive() const { return primitive_; }

 private:
  const PrimitiveType primitive_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_PRIMITIVE_SOLVER_H_
