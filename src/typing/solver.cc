#include "solver.h"

namespace darlang {
namespace typing {

Result TypeSolver::Solve(std::unique_ptr<Type>& out_type) {
  switch (class_) {
    case TypeClass::UNBOUND:
      return Result::Error(ErrorCode::TYPE_INDETERMINATE, "type class could not be inferred");
    case TypeClass::PRIMITIVE:
      out_type = std::make_unique<Primitive>(primitive_);
      return Result::Ok();
    case TypeClass::FUNCTION:
    {
      // TODO(acomminos): move this out
      if (!arguments_valid_) {
        return Result::Error(ErrorCode::TYPE_INDETERMINATE, "arguments unbound");
      }
      std::vector<std::unique_ptr<Type>> arg_types(arguments_.size());
      for (int i = 0; i < arguments_.size(); i++) {
        auto arg_result = arguments_[i]->Solver().Solve(arg_types[i]);
        if (!arg_result) {
          // TODO(acomminos): nest result
          return arg_result;
        }
      }

      if (!yields_) {
        return Result::Error(ErrorCode::TYPE_INDETERMINATE, "yield type unbound");
      }
      std::unique_ptr<Type> yield_type;
      auto yield_result = yields_->Solver().Solve(yield_type);
      if (!yield_result) {
        // TODO(acomminos): nest result
        return yield_result;
      }

      out_type = std::make_unique<Function>(std::move(arg_types), std::move(yield_type));
      return Result::Ok();
    }
  }
  return Result::Error(ErrorCode::UNIMPLEMENTED);
}

Result TypeSolver::Unify(TypeSolver& other) {
  // Handle a simple rejection where both classes are unbound and differ.
  if (other.type_class() != TypeClass::UNBOUND &&
      type_class() != TypeClass::UNBOUND &&
      other.type_class() != type_class())
  {
    return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "type classes have conflicting specializations");
  }

  // XXX(acomminos): bit of a hack until we get disjoint solvers.
  if (other.type_class() == TypeClass::PRIMITIVE) {
    primitive_ = other.primitive();
  }

  // If the other solver has valid arguments, we have to import them into this
  // solver.
  if (other.arguments_valid()) {
    auto& other_arguments = other.arguments();
    if (!arguments_valid()) {
      // Create typeables for this function's arguments if invalid.
      ConstrainArguments(other_arguments.size(), nullptr);
    }

    if (arguments_.size() != other_arguments.size()) {
      return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "argument size mismatch");
    }

    // Unify each corresponding argument.
    for (auto i = 0; i < arguments_.size(); i++) {
      if (!arguments_[i]->Unify(*other_arguments[i])) {
        return Result::Error(ErrorCode::TYPE_INCOMPATIBLE, "argument failed to unify");
      }
    }
  }

  // Unify against the yielded value of the function being unified.
  if (other.has_yields()) {
    auto yields = ConstrainYields(); // Implicitly creates yield value.
    bool result = yields->Unify(*other.ConstrainYields());
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

Result TypeSolver::ConstrainPrimitive(PrimitiveType primitive) {
  classify(TypeClass::PRIMITIVE);
  primitive_ = primitive;
  return Result::Ok();
}


Result TypeSolver::ConstrainArguments(int count, std::vector<std::shared_ptr<Typeable>>** out_args) {
  classify(TypeClass::FUNCTION);

  if (!arguments_valid_) {
    arguments_.reserve(count);
    for (int i = 0; i < count; i++) {
      arguments_.push_back(std::make_shared<Typeable>());
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

std::shared_ptr<Typeable> TypeSolver::ConstrainYields() {
  classify(TypeClass::FUNCTION);

  if (!yields_) {
    yields_ = std::make_shared<Typeable>();
  }
  return yields_;
}

}  // namespace typing
}  // namespace darlang
