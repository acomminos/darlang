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
  return false;
}

bool DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto func_typeable = std::make_unique<Typeable>();
  // TODO: transfer ownership of func_typeable to definition
  global_scope_.Assign(node.id, func_typeable.get());

  TypeableMap func_scope(&global_scope_);

  if (!func_typeable->Solver()->ConstrainArguments(node.args.size(), nullptr)) {
    assert(false); // FIXME(acomminos)
    return false;
  }

  auto expr_typeable = ExpressionTypeTransform(func_scope).Reduce(*node.expr);

  auto return_typeable = func_typeable->Solver()->ConstrainYields();
  if (!return_typeable->Unify(*expr_typeable)) {
    assert(false); // FIXME(acomminos)
    return false;
  }

  return false;
}

bool ExpressionTypeTransform::IdExpression(ast::IdExpressionNode& node) {
  auto id_typeable = std::make_unique<Typeable>();
  // TODO(acomminos): add to scope?
  set_result(std::move(id_typeable));
  return false;
}

bool ExpressionTypeTransform::IntegralLiteral(ast::IntegralLiteralNode& node) {
  auto int_typeable = std::make_unique<Typeable>();
  int_typeable->Solver()->ConstrainPrimitive(PrimitiveType::Int64);
  set_result(std::move(int_typeable));
  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
  auto call_typeable = std::make_unique<Typeable>();
  // TODO(acomminos)
  set_result(std::move(call_typeable));
  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = std::make_unique<Typeable>();
  for (auto& guard_case : node.cases) {
    auto case_typeable = ExpressionTypeTransform(scope_).Reduce(*guard_case.second);
    if (!guard_typeable->Unify(*case_typeable)) {
      assert(false); // FIXME(acomminos)
      return false;
    }
  }
  auto wildcard_typeable = ExpressionTypeTransform(scope_).Reduce(*node.wildcard_case);
  if (!guard_typeable->Unify(*wildcard_typeable)) {
    // TODO(acomminos)
    assert(false); // FIXME(acomminos)
    return false;
  }

  set_result(std::move(guard_typeable));
  return false;
}

}  // namespace typing
}  // namespace darlang
