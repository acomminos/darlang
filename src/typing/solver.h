#ifndef DARLANG_SRC_TYPING_SOLVER_H_
#define DARLANG_SRC_TYPING_SOLVER_H_

#include <cassert>
#include <memory>
#include <vector>
#include "errors.h"
#include "typing/types.h"

namespace darlang {
namespace typing {

class Typeable;
class TypeSolver;

// Holds either a TypeSolver or reference to a parent the typeable has been
// merged with.
class Typeable {
 public:
  // Instantiates a new unbound typeable.
  Typeable();
  // Unifies a typeable into this typeable, intersecting their type solvers.
  Result Unify(Typeable& other);
  // Obtains the TypeSolver determining this Typeable.
  TypeSolver* Solver();
 private:
  std::unique_ptr<TypeSolver> solver_;
  Typeable* parent_;
};

// A high-level classification of the type being solved for.
enum class TypeClass {
  UNBOUND = 0,
  PRIMITIVE, // Has primitive field data.
  FUNCTION,
};

// A TypeSolver is responsible for classifying and generating interfaces for
// types based on how they're used in the AST. The interface is designed to be
// simple to use while observing particular usages of an identifier.
class TypeSolver {
 public:
  TypeSolver() : class_(TypeClass::UNBOUND), arguments_valid_(false), yields_(nullptr) {}

  // Attempts to solve for a single concrete type from the constraints fed to
  // this type solver.
  //
  // The returned type is owned by this typeable, and is invalid after this
  // typeable is freed.
  Result Solve(Type*& out_type);

  // Merges constraints from the given solver into this solver.
  // Returns false if unification failed.
  Result Unify(TypeSolver& other);

  // Solves this TypeSolver as the given primitive.
  // Returns false if this causes a contradiction.
  Result ConstrainPrimitive(PrimitiveType primitive);

  // Returns a set of `count` arguments associated with the function.
  // If the arguments of this typeable have already been accessed with a
  // different cardinality, raises an error.
  // The typeable is implicitly specialized as a function.
  Result ConstrainArguments(int count, std::vector<Typeable>** out_args);

  // Returns the Typeable of the (implicitly-defined) return value.
  // TODO(acomminos): switch to return result
  Typeable* ConstrainYields();

  TypeClass type_class() const { return class_; }
  bool arguments_valid() const { return arguments_valid_; }
  bool has_yields() const { return !!yields_; }

 private:
  // Shifts the solver to the given TypeClass from the initial unbound state, or
  // throws an assertion if the solver is already specialized.
  inline void classify(TypeClass tc) {
    if (class_ == TypeClass::UNBOUND) {
      class_ = tc;
    }
    // TODO(acomminos): add error logging
    assert(class_ == tc);
  }

  std::vector<Typeable>& arguments() {
    assert(arguments_valid_);
    return arguments_;
  }

  std::unique_ptr<Type> solved_type_; // set after a successful Solve() call
  TypeClass class_;

  PrimitiveType primitive_;
  bool arguments_valid_; // true iff arguments has been constrained
  std::vector<Typeable> arguments_;
  std::unique_ptr<Typeable> yields_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_SOLVER_H_
