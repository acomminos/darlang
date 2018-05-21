#ifndef DARLANG_SRC_TYPING_TYPEABLE_H_
#define DARLANG_SRC_TYPING_TYPEABLE_H_

#include <memory>
#include "errors.h"

namespace darlang {
namespace typing {

class TypeSolver;

// A constrainable handle expected to resolve to a type after application of
// union-find to referencing AST nodes.
//
// shared_ptr is used rather than a single ownership hierarchy of unique_ptrs to
// handle typeables allocated by TypeSolvers. If two TypeSolvers get unified and
// one gets destroyed, the members/arguments/yield Typeables owned by the
// destroyed TypeSolver become invalid.
class Typeable : public std::enable_shared_from_this<Typeable> {
 public:
  // Instantiates a new unbound typeable.
  Typeable();
  // Unifies a typeable into this typeable, intersecting their type solvers.
  Result Unify(Typeable& other);
  // Obtains the TypeSolver determining this Typeable.
  TypeSolver& Solver();
 private:
  std::unique_ptr<TypeSolver> solver_;
  std::shared_ptr<Typeable> parent_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPEABLE_H_
