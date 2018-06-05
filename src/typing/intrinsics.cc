#include "typing/intrinsics.h"
#include "typing/function_solver.h"
#include "typing/primitive_solver.h"
#include "typing/function_specializer.h"

#include <cassert>

namespace darlang {
namespace typing {

void LoadIntrinsic(Intrinsic intrinsic, Specializer& spec) {
  switch (intrinsic) {
    case Intrinsic::IS:
    {
      auto supported_prims = {
        PrimitiveType::Int64,
        PrimitiveType::Boolean
      };

      for (auto arg_prim : supported_prims) {
        auto arg_typeable = Typeable::Create(
            std::make_unique<PrimitiveSolver>(arg_prim));
        auto return_typeable = Typeable::Create(
            std::make_unique<PrimitiveSolver>(PrimitiveType::Boolean));

        auto solver = std::make_unique<FunctionSolver>(2);
        assert(solver->args()[0]->Unify(arg_typeable));
        assert(solver->args()[1]->Unify(arg_typeable));
        assert(solver->yield()->Unify(return_typeable));

        spec.AddExternal("is", Typeable::Create(std::move(solver)));
      }
      break;
    }
    case Intrinsic::MOD:
    {
      // Only support integer modulo for the foreseeable future.
      auto arg_typeable = Typeable::Create(
          std::make_unique<PrimitiveSolver>(PrimitiveType::Int64));

      auto solver = std::make_unique<FunctionSolver>(2);
      assert(solver->args()[0]->Unify(arg_typeable));
      assert(solver->args()[1]->Unify(arg_typeable));
      assert(solver->yield()->Unify(arg_typeable));

      spec.AddExternal("mod", Typeable::Create(std::move(solver)));
      break;
    }
    default:
      assert(false);
      break;
  }
}

}  // namespace typing
}  // namespace darlang
