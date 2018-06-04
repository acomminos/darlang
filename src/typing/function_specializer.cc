#include "typing/function_specializer.h"

namespace darlang::typing {

using util::DeclarationMap;

/* static */
Result FunctionSpecializer::Specialize(
    std::string callee,
    const std::vector<TypeablePtr> args,
    const TypeablePtr yield,
    Logger& log,
    const DeclarationMap& decl_nodes,
    std::unordered_map<std::string, std::vector<Specialization>>& specializations) {

  // TODO(acomminos): only check this in debug mode?
  for (const auto& arg : args) {
    std::unique_ptr<Type> stub;
    // FIXME(acomminos): add a better way to determine if a typeable is
    //                   appropriately constrained- should all solvers be
    //                   solvable by definition? this makes sense, even for
    //                   tuple solving (where names can be undefined).
    if (!arg->Solve(stub)) {
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
  Specialization& spec = specs_[callee].emplace({}, args, yield);
  auto& node = decl_nodes[callee];

  FunctionSpecializer specializer(spec, log, decl_nodes, specs);
  node.Visit(specializer);

  if (!specializer.result()) {
    return specializer.result();
  }

  // TODO(acomminos): directly materialize type here?

  return Result::Ok();
}

FunctionSpecializer::FunctionSpecializer(Specialization& spec,
                                         Logger& log,
                                         const DeclarationMap& decl_nodes,
                                         std::unordered_map<Specialization, TypeableMap>& specs)
  : current_spec_(spec)
  , log_(log)
  , decl_nodes_(decl_nodes)
  , specs_(specs)
{
}

bool FunctionSpecializer::Declaration(DeclarationNode& node) {
  assert(current_spec_.args.size() == node.args.size());

  TypeableScope arg_scope;
  for (int i = 0; i < node.args.size(); i++) {
    std::string arg_name = node.args[i];
    arg_scope.Assign(arg_name, current_spec_.args[i]);
  }

  // Function-local and specialization-local typeables.
  //
  // We set this prior to performing any type substitution so that we can avoid
  // entering a cycle of callee resolution. As long as we resolve any bindings
  // before calls, we can easily exit a cycle by comparing specializations.
  TypeableMap& spec_types = current_spec_.typeables;
  ExpressionTypeTransform ett(log_, spec_types, arg_scope, specs_);
  auto func_typeable = ett.Annotate(node.expr);

  result_ = func_typeable.Unify(current_spec_.yield);

  return false;
}

}  // namespace darlang::typing
