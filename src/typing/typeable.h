#ifndef DARLANG_SRC_TYPING_TYPEABLE_H_
#define DARLANG_SRC_TYPING_TYPEABLE_H_

#include "errors.h"

namespace darlang {
namespace typing {

class TypeSolver;

// A constrainable handle expected to resolve to a type after application of
// union-find to referencing AST nodes.
class Typeable {
 public:
  // Instantiates a new unbound typeable.
  Typeable();
  // Unifies a typeable into this typeable, intersecting their type solvers.
  Result Unify(Typeable& other);
  // Obtains the TypeSolver determining this Typeable.
  TypeSolver& Solver();
 private:
  std::unique_ptr<TypeSolver> solver_;
  Typeable* parent_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPEABLE_H_
