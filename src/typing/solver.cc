#include "solver.h"

namespace darlang {
namespace typing {

Typeable::Typeable() : parent_(nullptr) {
  solver_ = std::make_unique<TypeSolver>();
}

Result Typeable::Unify(Typeable& other) {
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
  auto result = solver_->Unify(*other.solver_);
  if (result) {
    other.parent_ = this;
    other.solver_ = nullptr;
  }
  return result;
}

TypeSolver* Typeable::Solver() {
  if (parent_) {
    return parent_->Solver();
  }
  return solver_.get();
}

Result TypeSolver::Unify(TypeSolver& other) {
  // Handle a simple rejection where both classes are unbound and differ.
  if (other.type_class() != TypeClass::UNBOUND &&
      type_class() != TypeClass::UNBOUND &&
      other.type_class() != type_class())
  {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "type classes have conflicting specializations");
  }

  // If the other solver has valid arguments, we have to import them into this
  // solver.
  if (other.arguments_valid()) {
    auto& other_arguments = other.arguments();
    if (!arguments_valid()) {
      // Create typeables for this function's arguments if invalid.
      Arguments(other_arguments.size(), nullptr);
    }

    if (arguments_.size() != other_arguments.size()) {
      // TODO(acomminos): take note regarding cardinality mismatch
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "argument size mismatch");
    }

    // Unify each corresponding argument.
    for (int i = 0; i < arguments_.size(); i++) {
      if (!arguments_[i].Unify(other_arguments[i])) {
        // TODO(acomminos): log unification error
        return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "argument failed to unify");
      }
    }
  }

  // Unify against the yielded value of the function being unified.
  if (other.has_yields()) {
    auto yields = Yields(); // Implicitly creates yield value.
    bool result = yields->Unify(*other.Yields());
    if (!result) {
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "return value failed to unify");
    }
  }

  // If the other solver has a valid class, specialize this solver to it.
  if (other.type_class() != TypeClass::UNBOUND) {
    classify(other.type_class());
  }

  return Result::Ok();
}

Result TypeSolver::Primitive(PrimitiveType primitive) {
  classify(TypeClass::PRIMITIVE);
  primitive_ = primitive;
  return Result::Ok();
}


Result TypeSolver::Arguments(int count, std::vector<Typeable>** out_args) {
  classify(TypeClass::FUNCTION);

  if (!arguments_valid_) {
    arguments_.reserve(count);
    for (int i = 0; i < count; i++) {
      arguments_.push_back(Typeable());
    }
    arguments_valid_ = true;
  }

  if (arguments_.size() != count) {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "argument count conflict");
  }

  if (out_args) {
    *out_args = &arguments_;
  }
  return Result::Ok();
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
