#include "typing/intrinsics.h"
#include "typing/function_solver.h"
#include "typing/primitive_solver.h"
#include "typing/function_specializer.h"

#include <cassert>

namespace darlang {
namespace typing {

template <typename ... Args>
static TypeablePtr CreatePrimitiveFunction(PrimitiveType yield, Args... args) {
  const std::vector<PrimitiveType> arg_vector = {args...};
  auto solver = std::make_unique<FunctionSolver>(arg_vector.size());

  int arg_index = 0;
  for (const auto arg_prim : arg_vector) {
    auto arg_type = Typeable::Create(std::make_unique<PrimitiveSolver>(arg_prim));
    assert(solver->args()[arg_index++]->Unify(arg_type));
  }

  auto yield_type = Typeable::Create(std::make_unique<PrimitiveSolver>(yield));
  assert(solver->yield()->Unify(yield_type));

  return Typeable::Create(std::move(solver));
}

void LoadIntrinsic(Intrinsic intrinsic, Specializer& spec) {
  switch (intrinsic) {
    case Intrinsic::IS:
    {
      auto supported_prims = {
        PrimitiveType::Int64,
        PrimitiveType::Boolean
      };

      for (auto arg_prim : supported_prims) {
        TypeablePtr type = CreatePrimitiveFunction(PrimitiveType::Boolean, arg_prim, arg_prim);
        spec.AddExternal("is", type);
      }
      break;
    }
    case Intrinsic::MOD:
    {
      // Only support integer modulo for the foreseeable future.
      PrimitiveType prim = PrimitiveType::Int64;
      TypeablePtr type = CreatePrimitiveFunction(prim, prim, prim);
      spec.AddExternal("mod", type);
      break;
    }
    case Intrinsic::ADD:
    {
      // Only support integer addition for now.
      PrimitiveType prim = PrimitiveType::Int64;
      TypeablePtr type = CreatePrimitiveFunction(prim, prim, prim);
      spec.AddExternal("add", type);
      break;
    }
    default:
      assert(false);
      break;
  }
}

}  // namespace typing
}  // namespace darlang
