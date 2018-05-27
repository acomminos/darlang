#ifndef DARLANG_SRC_TYPING_FUNCTION_SOLVER_H_
#define DARLANG_SRC_TYPING_FUNCTION_SOLVER_H_

#include "typing/solver.h"

namespace darlang {
namespace typing {

class FunctionSolver : public Solver {
 public:
   // Creates a new function solver with the given number of arguments.
   // Allocates typeables for each argument, as well as the return value.
   FunctionSolver(int num_args);

   Result Merge(Solver& solver) override { return solver.MergeInto(*this); }
   Result MergeInto(FunctionSolver& other) override;
   Result Solve(std::unique_ptr<Type>& out_type) override;

   int num_args() const { return args_.size(); }
   const std::vector<TypeablePtr>& args() const { return args_; }
   const TypeablePtr yield() { return yield_; }

 private:
   std::vector<TypeablePtr> args_;
   const TypeablePtr yield_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_FUNCTION_SOLVER_H_
