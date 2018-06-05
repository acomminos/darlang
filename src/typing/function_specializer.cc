#include "typing/function_specializer.h"
#include "typing/function_solver.h"

namespace darlang::typing {

using util::DeclarationMap;

Specializer::Specializer(Logger& log, const util::DeclarationMap decl_nodes)
  : log_(log), decl_nodes_(decl_nodes) {
}

Result Specializer::Specialize(std::string callee,
                               std::vector<TypeablePtr> args,
                               TypeablePtr& out_yield) {
  auto solver = std::make_unique<FunctionSolver>(args.size());
  TypeablePtr func_yield = solver->yield();

  for (int i = 0; i < args.size(); i++) {
    const auto& arg = args[i];
    // FIXME(acomminos): add a better way to determine if a typeable is
    //                   appropriately constrained- should all solvers be
    //                   solvable by definition? this makes sense, even for
    //                   tuple solving (where names can be undefined).
    if (!arg->IsSolvable()) {
      return Result::Error(ErrorCode::TYPE_INDETERMINATE, "attempted to specialize with unsolved arg");
    }

    // All arguments should unify into our new solver, which is unconstrained.
    assert(solver->args()[i]->Unify(arg));
  }

  // Upon successful unification, the solver's yield value will be unioned with
  // func_yield.
  out_yield = func_yield;

  // Use a new function solver backed typeable.
  TypeablePtr func_typeable = Typeable::Create(std::move(solver));

  // Attempt to unify against all known specializations for this callee.
  // It's not possible for us to unify against an unspecialized set of
  // arguments, since we require solvable arguments as a precondition for
  // specialization.
  if (specs_.Unify(callee, func_typeable)) {
    return Result::Ok();
  }

  // If we failed to find an existing specialization for the given args, create
  // a new one with the arguments provided.
  auto& spec = specs_.Add(callee, {{}, func_typeable});

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

Result Specializer::AddExternal(std::string callee, TypeablePtr func_typeable) {
  if (!func_typeable->IsSolvable()) {
    return Result::Error(ErrorCode::TYPE_INDETERMINATE, "attempted to specialize with unsolvable typeable");
  }

  specs_.Add(callee, {{}, func_typeable});

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
  auto solver = std::make_unique<FunctionSolver>(node.args.size());
  std::vector<TypeablePtr> args = solver->args();
  TypeablePtr yield = solver->yield();

  auto func_typeable = Typeable::Create(std::move(solver));
  assert(spec_.func_typeable->Unify(func_typeable));

  TypeableScope arg_scope;
  for (int i = 0; i < node.args.size(); i++) {
    std::string arg_name = node.args[i];
    arg_scope.Assign(arg_name, args[i].get());
  }

  // Function-local and specialization-local typeables.
  //
  // We set this prior to performing any type substitution so that we can avoid
  // entering a cycle of callee resolution. As long as we resolve any bindings
  // before calls, we can easily exit a cycle by comparing specializations.
  TypeableMap& spec_types = spec_.typeables;
  ExpressionTypeTransform ett(log_, spec_types, arg_scope, specializer_);
  auto expr_typeable = ett.Annotate(*node.expr);

  result_ = expr_typeable->Unify(yield);

  return false;
}

}  // namespace darlang::typing
