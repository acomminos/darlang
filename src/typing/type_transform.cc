#include "type_transform.h"

#include <cassert>

namespace darlang {
namespace typing {

void TypeTransform::Module(ast::ModuleNode& node) {
  TypeableMap global_scope;
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(global_scope);
    child->Visit(decl_transform);
  }
  set_result(global_scope);
}

void DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto func_typeable = std::make_unique<Typeable>();
  // TODO: transfer ownership of func_typeable to definition
  global_scope_.Assign(node.id, func_typeable.get());

  TypeableMap func_scope(&global_scope_);
  auto& args = func_typeable->Arguments(

  auto expr_typeable = ExpressionTypeTransform(func_scope).Reduce(*node.expr);
  assert(expr_typeable);
  assert(func_typeable.Yields()->Unify(expr_typeable));
}

void ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
}

void ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = std::make_unique<Typeable>();
  for (auto& guard_case : node.cases) {
  }
}

}  // namespace typing
}  // namespace darlang
