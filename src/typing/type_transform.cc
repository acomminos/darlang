#include "type_transform.h"

namespace darlang {
namespace typing {

void TypeTransform::Module(ast::ModuleNode& node) {
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(type_system_, globals_);
    child->Visit(decl_transform);
  }
}

void DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto& func_typeable = type_system_.CreateTypeable();

  std::unordered_map<std::string, Typeable*> arg_scope_;
  int arg_idx = 0;
  for (auto& arg : node.args) {
    arg_scope_[arg.id] = func_typeable.Argument(arg_idx++, arg.id);
  }

  ExpressionTypeTransform expr_transform(arg_scope_, globals_);
  node.expr->Visit(expr_transform);
  assert(expr_transform.result());

  func_typeable.Returns(expr_transform.result());
}

void ExpressionTypeTransform::Invocation(ast::GuardNode& node) {
}

void ExpressionTypeTransform::Guard(ast::GuardNode& node) {
}

}  // namespace typing
}  // namespace darlang
