#include "type_transform.h"

#include <cassert>

namespace darlang {
namespace typing {

bool TypeTransform::Module(ast::ModuleNode& node) {
  TypeableMap global_scope;
  // TODO(acomminos): break this into a spearate pass so we toplevel functions
  //                  can mutually reference each other
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(global_scope);
    child->Visit(decl_transform);
  }
  set_result(global_scope);
  return false;
}

bool DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto func_typeable = std::make_unique<Typeable>();
  // TODO: transfer ownership of func_typeable to definition
  global_scope_.Assign(node.id, func_typeable.get());

  TypeableMap func_scope(&global_scope_);
  auto& args = func_typeable->Solver()->Arguments(node.args.size());

  auto expr_typeable = ExpressionTypeTransform(func_scope).Reduce(*node.expr);
  if (!expr_typeable) {
    return false;
  }

  assert(expr_typeable);
  assert(func_typeable->Solver()->Yields()->Unify(*expr_typeable));
  return false;
}

bool ExpressionTypeTransform::IdExpression(ast::IdExpressionNode& node) {
  // TODO: associate typeable with the node.
  auto id_typeable = std::make_unique<Typeable>();
  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
  // TODO(acomminos)
  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = std::make_unique<Typeable>();
  for (auto& guard_case : node.cases) {
  }
  return false;
}

}  // namespace typing
}  // namespace darlang
