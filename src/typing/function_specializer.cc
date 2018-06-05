#include "typing/function_specializer.h"

namespace darlang::typing {

using util::DeclarationMap;

Specializer::Specializer(Logger& log, const util::DeclarationMap decl_nodes)
  : log_(log), decl_nodes_(decl_nodes) {
}

Result Specializer::Specialize(std::string callee,
                               std::vector<TypeablePtr> args,
                               TypeablePtr yield) {
  // TODO(acomminos): only check this in debug mode?
  for (const auto& arg : args) {
    // FIXME(acomminos): add a better way to determine if a typeable is
    //                   appropriately constrained- should all solvers be
    //                   solvable by definition? this makes sense, even for
    //                   tuple solving (where names can be undefined).
    if (!arg->IsSolvable()) {
      return Result::Error(ErrorCode::TYPE_INDETERMINATE, "attempted to specialize with unsolved arg");
    }
  }

  // Attempt to unify against all known specializations for this callee.
  // It's not possible for us to unify against an unspecialized set of
  // arguments, since we require solvable arguments as a precondition for
  // specialization.
  for (const auto& spec : specs_[callee]) {
    bool spec_success = true;
    assert(args.size() == spec.args.size());
    for (int i = 0; i < args.size(); i++) {
      // TODO(acomminos): attempt unification idempotently/transactionally?
      //                  although greedy unification may not affect the output,
      //                  it's still weird to have one callee argument unify
      //                  against multiple specializations' typeables.
      if (!args[i]->Unify(spec.args[i])) {
        spec_success = false;
        break;
      }
    }

    if (spec_success) {
      // After unifying all arguments successfully, we can unify the yielded
      // value and return.
      Result res;
      if (!(res = yield->Unify(spec.yield))) {
        return res;
      }
      return Result::Ok();
    }
  }

  // If we failed to find an existing specialization for the given args, create
  // a new one with the arguments provided.
  specs_[callee].push_back({{}, args, yield});
  Specialization& spec = specs_[callee].back();

  // We can only instantiate a new specialization of a function if it was
  // defined in this module. Otherwise (e.g. for intrinsics, external
  // references) their code is already generated and we cannot continue.
  auto node = decl_nodes_.find(callee);
  if (node == decl_nodes_.end()) {
    return Result::Error(ErrorCode::ID_UNDECLARED, "could not specialize external function " + callee);
  }

  FunctionSpecializer func_specializer(log_, *this, spec);
  node->second->Visit(func_specializer);

  if (!func_specializer.result()) {
    return func_specializer.result();
  }

  // TODO(acomminos): directly materialize type here?

  return Result::Ok();
}

Result Specializer::AddExternal(std::string callee, std::vector<TypeablePtr> args, TypeablePtr yield) {
  for (const auto& arg : args) {
    if (!arg->IsSolvable()) {
      return Result::Error(ErrorCode::TYPE_INDETERMINATE, "attempted to specialize with unsolved arg");
    }
  }
  if (!yield->IsSolvable()) {
    return Result::Error(ErrorCode::TYPE_INDETERMINATE, "attempted to specialize with unsolved yield");
  }

  specs_[callee].push_back({{}, args, yield});

  return Result::Ok();
}

FunctionSpecializer::FunctionSpecializer(Logger& log,
                                         Specializer& specializer,
                                         Specialization& spec)
  : log_(log)
  , specializer_(specializer)
  , spec_(spec)
{
}

bool FunctionSpecializer::Declaration(ast::DeclarationNode& node) {
  assert(spec_.args.size() == node.args.size());

  TypeableScope arg_scope;
  for (int i = 0; i < node.args.size(); i++) {
    std::string arg_name = node.args[i];
    arg_scope.Assign(arg_name, spec_.args[i].get());
  }

  // Function-local and specialization-local typeables.
  //
  // We set this prior to performing any type substitution so that we can avoid
  // entering a cycle of callee resolution. As long as we resolve any bindings
  // before calls, we can easily exit a cycle by comparing specializations.
  TypeableMap& spec_types = spec_.typeables;
  ExpressionTypeTransform ett(log_, spec_types, arg_scope, specializer_);
  auto func_typeable = ett.Annotate(*node.expr);

  result_ = func_typeable->Unify(spec_.yield);

  return false;
}

}  // namespace darlang::typing
