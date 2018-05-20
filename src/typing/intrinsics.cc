#include "typing/intrinsics.h"

namespace darlang {
namespace typing {

std::unique_ptr<Typeable> CreateIntrinsicTypeable(Intrinsic intrinsic) {
  switch (intrinsic) {
    case Intrinsic::IS:
    {
      auto typeable = std::make_unique<Typeable>();
      std::vector<Typeable>* args;

      assert(typeable->Solver()->ConstrainArguments(2, &args));
      assert(args[0].Unify(args[1])); // Require an `is` comparison to have the same type.
      assert(typeable->Solver()->Yields()->ConstrainPrimitive(PrimitiveType::Boolean));

      return std::move(typeable);
    }
    case Intrinsic::MOD:
    {
      auto typeable = std::make_unique<Typeable>();
      std::vector<Typeable>* args;

      // Ensure the two arguments and return value all have the same type.
      //
      // XXX(acomminos): Have the LLVM backend throw an error if the
      // materialized primitive type is not supported, at least until we can
      // represent union types in our constraint solver.
      assert(typeable->Solver()->ConstrainArguments(2, &args));
      assert(args[0].Unify(args[1]));
      assert(typeable->Solver()->Yields()->Unify(args[0]));

      return std::move(typeable);
    }
    default:
      return nullptr;
  }
}

}  // namespace typing
}  // namespace darlang
