#include "type_transform.h"

#include <cassert>
#include "intrinsics.h"
#include "logger.h"

namespace darlang {
namespace typing {

bool TypeTransform::Module(ast::ModuleNode& node) {
  TypeableMap typeables;
  TypeableScope global_scope;

  // TODO(acomminos): break this into a separate pass so toplevel functions
  //                  can mutually reference each other
  for (auto& child : node.body) {
    DeclarationTypeTransform decl_transform(log_, typeables, global_scope);
    child->Visit(decl_transform);
  }

  // After building up typeable constraints, attempt to solve for concrete types.
  TypeMap types;
  bool error = false;
  for (auto& map_pair : typeables) {
    auto& node_id = map_pair.first;
    auto& typeable = map_pair.second;
    auto result = typeable->Solver().Solve(types[node_id]);
    if (!result) {
      // TODO(acomminos): add location info
      log_.Error(result, {});
      error = true;
    }
  }

  if (error) {
    log_.Fatal("type check stage failed, stopping", node.start);
  }

  set_result(std::move(types));
  return false;
}

bool DeclarationTypeTransform::Declaration(ast::DeclarationNode& node) {
  auto func_typeable = std::make_shared<Typeable>();
  global_scope_.Assign(node.name, func_typeable.get());

  std::vector<std::shared_ptr<Typeable>>* arg_typeables;
  assert(func_typeable->Solver().ConstrainArguments(node.args.size(), &arg_typeables));

  TypeableScope func_scope(&global_scope_);
  // Create typeable declarations for all argument identifiers.
  for (int i = 0; i < node.args.size(); i++) {
    func_scope.Assign(node.args[i], (*arg_typeables)[i].get());
  }

  auto expr_typeable = ExpressionTypeTransform(log_, typeables_, func_scope).Annotate(*node.expr);
  assert(expr_typeable);

  auto return_typeable = func_typeable->Solver().ConstrainYields();
  assert(return_typeable->Unify(*expr_typeable));

  typeables_[node.id] = func_typeable;

  return false;
}

std::shared_ptr<Typeable> ExpressionTypeTransform::AnnotateChild(ast::Node& node, const TypeableScope& scope) {
  return ExpressionTypeTransform(log_, map(), scope).Annotate(node);
}

bool ExpressionTypeTransform::IdExpression(ast::IdExpressionNode& node) {
  auto id_typeable = std::make_shared<Typeable>();

  auto scope_typeable = scope_.Lookup(node.name);
  // No forward declarations permitted.
  if (!scope_typeable) {
    auto result = Result::Error(ErrorCode::ID_UNDECLARED,
                                "undeclared identifier '" + node.name + "' referenced");
    log_.Fatal(result, node.start);
  }

  Result result;
  if (!(result = scope_typeable->Unify(*id_typeable))) {
    log_.Fatal(result, node.start);
  }

  set_result(id_typeable);

  return false;
}

bool ExpressionTypeTransform::IntegralLiteral(ast::IntegralLiteralNode& node) {
  auto int_typeable = std::make_shared<Typeable>();

  Result result;
  if (!(result = int_typeable->Solver().ConstrainPrimitive(PrimitiveType::Int64))) {
    log_.Fatal(result, node.start);
  }

  set_result(int_typeable);

  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node) {
  Result result;
  // Constrain the function typeable associated with the callee.
  // TODO(acomminos): have prepass populate toplevel functions/identifiers
  auto func_typeable = scope_.Lookup(node.callee);
  if (!func_typeable) {
    // Attempt to resolve an intrinsic if we couldn't find the callee in scope.
    // XXX(acomminos): should we always try to resolve intrinsics first?
    if (GetIntrinsic(node.callee) != Intrinsic::UNKNOWN) {
      // FIXME(acomminos): leave intrinsics unbound for now.
      //                   progress towards templating is in
      //                   typing/intrinsics.{h,cc}
      auto stub_typeable = std::make_shared<Typeable>();
      set_result(stub_typeable);
      return false;
    }
    log_.Fatal(Result::Error(ErrorCode::ID_UNDECLARED, "undeclared function " + node.callee),
               node.start);
    assert(false);
  }

  std::vector<std::shared_ptr<Typeable>>* args;
  assert(func_typeable->Solver().ConstrainArguments(node.args.size(), &args));

  for (int i = 0; i < args->size(); i++) {
    auto& arg_typeable = *(*args)[i];
    auto& arg_expr = node.args[i];
    // Unify the callee's argument typeable against the expression used.
    auto arg_expr_typeable = AnnotateChild(*arg_expr);

    if (!(result = arg_typeable.Unify(*arg_expr_typeable))) {
      log_.Fatal(result, node.start);
    }
  }

  auto yield_typeable = std::make_shared<Typeable>();
  if (!(result = func_typeable->Solver().ConstrainYields()->Unify(*yield_typeable))) {
    log_.Fatal(result, node.start);
  }

  set_result(yield_typeable);

  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node) {
  auto guard_typeable = std::make_shared<Typeable>();
  for (auto& guard_case : node.cases) {
    auto case_typeable = AnnotateChild(*guard_case.second);
    assert(guard_typeable->Unify(*case_typeable));
  }
  auto wildcard_typeable = AnnotateChild(*node.wildcard_case);
  assert(guard_typeable->Unify(*wildcard_typeable));

  set_result(guard_typeable);

  return false;
}

bool ExpressionTypeTransform::Bind(ast::BindNode& node) {
  TypeableScope bind_scope(&scope_);

  // Compute the type of the identifier-bound expression, and use it in the
  // scope of the following body.
  auto expr_typeable = AnnotateChild(*node.expr);
  // TODO(acomminos): throw error if name is already bound?
  bind_scope.Assign(node.identifier, expr_typeable.get());

  auto body_typeable = AnnotateChild(*node.body, bind_scope);

  auto typeable = std::make_shared<Typeable>();
  assert(typeable->Unify(*body_typeable));

  set_result(typeable);

  return false;
}

}  // namespace typing
}  // namespace darlang
