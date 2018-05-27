#ifndef DARLANG_SRC_TYPING_SOLVER_H_
#define DARLANG_SRC_TYPING_SOLVER_H_

#include <cassert>
#include <memory>
#include <vector>
#include "errors.h"
#include "typing/types.h"
#include "typing/typeable.h"

namespace darlang {
namespace typing {

// A high-level classification of the type being solved for.
enum class TypeClass {
  UNBOUND = 0,
  PRIMITIVE, // Has primitive field data.
  FUNCTION,
};

class FunctionSolver;
class TupleSolver;
class PrimitiveSolver;

class Solver {
 public:
  // Double-dispatch mechanism to delegate constraint union to the given solver
  // implementation. Returns an error on failure.
  virtual Result Merge(Solver& solver) = 0;
  // Attempts to materialize a type based on the constraints known to the
  // implementation. Stores the synthesized type into `out_type` on success.
  virtual Result Solve(std::unique_ptr<Type>& out_type) = 0;

  // Implementation-specific unification methods to merge the callee object
  // into the provided solver. Compatible solvers should override the
  // appropriate methods, exporting their constraints into the passed solver.
  virtual Result MergeInto(FunctionSolver& func_solver) {
    return error_incompatible();
  }
  virtual Result MergeInto(TupleSolver& func_solver) {
    return error_incompatible();
  }
  virtual Result MergeInto(PrimitiveSolver& func_solver) {
    return error_incompatible();
  }

 private:
  // Convenience function to produce a result indicating invalid unification.
  static inline Result error_incompatible() {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "attempted to unify unsupported type classes");
  }
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_SOLVER_H_
