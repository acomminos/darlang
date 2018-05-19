#include "type_transform.h"

#include <cassert>
#include <iostream>

namespace darlang {
namespace typing {

bool TypeTransform::Module(ast::ModuleNode& node) {
  TypeableMap typeables;
  TypeableScope global_scope;
  // TODO(acomminos): break this into a separate pass so toplevel functions
  //                  can mutually reference each other
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(typeables, global_scope);
    child->Visit(decl_transform);
  }
  set_result(std::move(typeables));
  return false;
}

bool DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto func_typeable = std::make_unique<Typeable>();
  global_scope_.Assign(node.name, func_typeable.get());

  std::vector<Typeable>* arg_typeables;
  assert(func_typeable->Solver()->ConstrainArguments(node.args.size(), &arg_typeables));

  TypeableScope func_scope(&global_scope_);
  // Create typeable declarations for all argument identifiers.
  for (int i = 0; i < node.args.size(); i++) {
    // FIXME(acomminos): this only works at the moment because the vector is not
    // reallocated. fix this, move to unique_ptrs for the child typeables.
    func_scope.Assign(node.args[i], &(*arg_typeables)[i]);
  }

  auto expr_typeable = ExpressionTypeTransform(typeables_, func_scope).Reduce(*node.expr);

  auto return_typeable = func_typeable->Solver()->ConstrainYields();
  assert(return_typeable->Unify(*expr_typeable));

  typeables_[node.id] = std::move(func_typeable);

  return false;
}

bool ExpressionTypeTransform::IdExpression(ast::IdExpressionNode& node) {
  auto id_typeable = std::make_unique<Typeable>();

  auto scope_typeable = scope_.Lookup(node.name);
  // No forward declarations permitted.
  // FIXME(acomminos): error propagation.
  assert(scope_typeable);

  assert(scope_typeable->Unify(*id_typeable));

  set_result(id_typeable.get());
  typeables_[node.id] = std::move(id_typeable);

  return false;
}

bool ExpressionTypeTransform::IntegralLiteral(ast::IntegralLiteralNode& node) {
  auto int_typeable = std::make_unique<Typeable>();
  int_typeable->Solver()->ConstrainPrimitive(PrimitiveType::Int64);

  set_result(int_typeable.get());
  typeables_[node.id] = std::move(int_typeable);

  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
  // Constrain the function typeable associated with the callee.
  // TODO(acomminos): have prepass populate toplevel functions/identifiers
  auto func_typeable = scope_.Lookup(node.callee);
  assert(func_typeable);

  std::vector<Typeable>* args;
  assert(func_typeable->Solver()->ConstrainArguments(node.args.size(), &args));

  for (int i = 0; i < args->size(); i++) {
    auto& arg_typeable = (*args)[i];
    auto& arg_expr = node.args[i];
    // Unify the callee's argument typeable against the expression used.
    auto arg_expr_typeable = ExpressionTypeTransform(typeables_, scope_).Reduce(*arg_expr);
    assert(arg_typeable.Unify(*arg_expr_typeable));
  }

  auto yield_typeable = std::make_unique<Typeable>();
  assert(func_typeable->Unify(*yield_typeable));

  set_result(yield_typeable.get());
  typeables_[node.id] = std::move(yield_typeable);

  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = std::make_unique<Typeable>();
  for (auto& guard_case : node.cases) {
    auto case_typeable = ExpressionTypeTransform(typeables_, scope_).Reduce(*guard_case.second);
    assert(guard_typeable->Unify(*case_typeable));
  }
  auto wildcard_typeable = ExpressionTypeTransform(typeables_, scope_).Reduce(*node.wildcard_case);
  assert(guard_typeable->Unify(*wildcard_typeable));

  set_result(guard_typeable.get());
  typeables_[node.id] = std::move(guard_typeable);

  return false;
}

}  // namespace typing
}  // namespace darlang
