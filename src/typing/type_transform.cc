#include "type_transform.h"

#include <cassert>

namespace darlang {
namespace typing {

void TypeTransform::Module(ast::ModuleNode& node) {
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(type_system_, global_scope_);
    child->Visit(decl_transform);
  }
}

void DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto& func_typeable = type_system_.CreateTypeable();
  global_scope_.Assign(node.id, func_typeable);

  TypeableMap func_scope(&global_scope_);
  int arg_idx = 0;
  for (auto& arg : node.args) {
    func_scope.Assign(arg, func_typeable.Argument(arg_idx++, arg));
  }

  auto expr_typeable = ExpressionTypeTransform::Transform(type_system_, func_scope, *node.expr);
  assert(expr_typeable);

  func_typeable.Returns(expr_typeable);
}

/* static */
Typeable* ExpressionTypeTransform::Transform(
    TypeSystem& type_system,
    const TypeableMap& scope,
    ast::Node& node)
{
  ExpressionTypeTransform transform(type_system, scope);
  node.Visit(transform);
  assert(transform.result());
  return transform.result();
}

void ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
}

void ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = type_system_.CreateTypeable();
  for (auto& guard_case : node.cases) {
  }
}

}  // namespace typing
}  // namespace darlang
