#include "solver.h"

namespace darlang {
namespace typing {

Typeable::Typeable() {
  solver_ = std::make_unique<TypeSolver>();
}

bool Typeable::Unify(Typeable& other) {
  // Traverse to the roots of each of the typeables being merged.
  if (parent_) {
    return parent_->Unify(other);
  }
  if (other.parent_) {
    return Unify(*(other.parent_));
  }

  // If we're the highest-level parents, expect a solver implementation.
  assert(solver_);
  assert(other.solver_);
  bool success = solver_->Unify(*other.solver_);
  if (success) {
    other.parent_ = this;
    other.solver_ = nullptr;
  }
  return success;
}

TypeSolver* Typeable::Solver() {
  if (parent_) {
    return parent_->Solver();
  }
  return solver_.get();
}

bool TypeSolver::Unify(TypeSolver& other) {
  // Handle a simple rejection where both classes are unbound and differ.
  if (other.type_class() != TypeClass::UNBOUND &&
      type_class() != TypeClass::UNBOUND &&
      other.type_class() != type_class())
  {
    return false;
  }

  // If the other solver has a valid class, specialize this solver to it.
  if (other.type_class() != TypeClass::UNBOUND) {
    classify(other.type_class());
  }

  // If the other solver has valid arguments, we have to import them into this
  // solver.
  if (other.arguments_valid()) {
    auto& other_arguments = other.arguments();
    if (!arguments_valid()) {
      // Create typeables for this function's arguments if invalid.
      Arguments(other_arguments.size());
    }

    if (arguments_.size() != other_arguments.size()) {
      // TODO(acomminos): take note regarding cardinality mismatch
      return false;
    }

    // Unify each corresponding argument.
    for (int i = 0; i < arguments_.size(); i++) {
      if (!arguments_[i].Unify(other_arguments[i])) {
        // TODO(acomminos): log unification error
        return false;
      }
    }
  }

  // Unify against the yielded value of the function being unified.
  if (other.has_yields()) {
    auto yields = Yields(); // Implicitly creates yield value.
    yields->Unify(*other.Yields());
  }
}

bool TypeSolver::Primitive(PrimitiveType primitive) {
  classify(TypeClass::PRIMITIVE);
  primitive_ = primitive;
  return true;
}


std::vector<Typeable>& TypeSolver::Arguments(int count) {
  classify(TypeClass::FUNCTION);

  if (!arguments_valid_) {
    arguments_.reserve(count);
    for (int i = 0; i < count; i++) {
      arguments_[i] = Typeable();
    }
    arguments_valid_ = true;
  }

  // TODO(acomminos): more verbose error logging
  assert(arguments_.size() == count);
  return arguments_;
}

Typeable* TypeSolver::Yields() {
  classify(TypeClass::FUNCTION);

  if (!yields_) {
    yields_ = std::make_unique<Typeable>();
  }
  return yields_.get();
}

}  // namespace typing
}  // namespace darlang
