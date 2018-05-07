#ifndef DARLANG_SRC_TYPING_SOLVER_H_
#define DARLANG_SRC_TYPING_SOLVER_H_

#include "types.h"

namespace darlang {
namespace typing {

class Typeable;

// A registry of predefined types.
class TypeSystem {
 private:
  StructDef user_structs_;
  FuncDef user_funcs_;
};

// Holds either a TypeSolver or reference to a parent the typeable has been
// merged with.
class Typeable {
 public:
  // Unifies a typeable into this typeable, intersecting their type solvers.
  void Unify(Typeable& other);
 private:
  std::unique_ptr<TypeSolver> solver_;
  Typeable* parent_;
};

// A TypeSolver is responsible for deriving composite and singular types.
// Subtypes of the type being built may be either named or indexed. These
// subtypes have their own groups/solvers.
//
// Examples:
//  Struct: (f1:string) -> (f2:string) -> struct
//  Function: (arg1:int64) -> string
//  Primitive: int64
//
class TypeSolver {
 public:
  TypeSolver(TypeSystem& system) : system_(system) {}

  Type Solve();

  // Merges constraints from the given solver into this solver.
  void Intersect(TypeSolver& solver);

  // Adds a constraint to the parent type.
  void Constrain(TypeConstraint constraint);

  // Returns the Typeable for a member's identifier.
  // Implicitly defines a member with the provided identifier if not present,
  // and constrains the parent type to be a struct.
  Typeable* Member(const std::string id);

  // Returns the Typeable for the argument at the given index.
  // Implicitly produces a solvable type at this position, and constrains the
  // parent type to be a function.
  Typeable* Argument(int index);

  // Returns the Typeable of the return value.
  Typeable* Yields();

 private:
  TypeSystem& system_;

  std::vector<TypeConstraint> constraints_;
  std::unordered_map<int, Typeable*> arguments_;
  std::unordered_map<std::string, Typeable*> members_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_SOLVER_H_
