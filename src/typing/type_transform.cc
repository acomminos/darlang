#include "type_transform.h"

#include <cassert>
#include "intrinsics.h"
#include "typing/function_solver.h"
#include "typing/primitive_solver.h"
#include "typing/typeable.h"

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
    auto result = typeable->Solve(types[node_id]);
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
  auto decl_solver = std::make_unique<FunctionSolver>(node.args.size());
  auto decl_args = decl_solver->args();
  auto decl_yield = decl_solver->yield();

  auto func_typeable = Typeable::Create(std::move(decl_solver));
  global_scope_.Assign(node.name, func_typeable.get());

  TypeableScope func_scope(&global_scope_);
  // Create typeable declarations for all argument identifiers.
  for (int i = 0; i < node.args.size(); i++) {
    func_scope.Assign(node.args[i], decl_args[i].get());
  }

  auto expr_typeable = ExpressionTypeTransform(log_, typeables_, func_scope).Annotate(*node.expr);
  assert(expr_typeable);

  assert(decl_yield->Unify(expr_typeable));

  // TODO(acomminos): declare func_typeable first before recursing
  typeables_[node.id] = func_typeable;

  return false;
}

TypeablePtr ExpressionTypeTransform::AnnotateChild(ast::Node& node, const TypeableScope& scope) {
  return ExpressionTypeTransform(log_, annotations(), scope).Annotate(node);
}

bool ExpressionTypeTransform::IdExpression(ast::IdExpressionNode& node, TypeablePtr& out_typeable) {
  auto id_typeable = Typeable::Create();

  auto scope_typeable = scope_.Lookup(node.name);
  // No forward declarations permitted.
  if (!scope_typeable) {
    auto result = Result::Error(ErrorCode::ID_UNDECLARED,
                                "undeclared identifier '" + node.name + "' referenced");
    log_.Fatal(result, node.start);
  }

  Result result;
  if (!(result = scope_typeable->Unify(id_typeable))) {
    log_.Fatal(result, node.start);
  }

  out_typeable = id_typeable;

  return false;
}

bool ExpressionTypeTransform::IntegralLiteral(ast::IntegralLiteralNode& node, TypeablePtr& out_typeable) {
  auto int_solver = std::make_unique<PrimitiveSolver>(PrimitiveType::Int64);
  out_typeable = Typeable::Create(std::move(int_solver));
  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node, TypeablePtr& out_typeable) {
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
      out_typeable = Typeable::Create();
      return false;
    }
    log_.Fatal(Result::Error(ErrorCode::ID_UNDECLARED, "undeclared function " + node.callee),
               node.start);
  }

  // Unify each call against the underlying declaration type.
  auto call_solver = std::make_unique<FunctionSolver>(node.args.size());
  for (int i = 0; i < call_solver->args().size(); i++) {
    auto& arg_typeable = call_solver->args()[i];
    auto& arg_expr = node.args[i];
    // Unify the callee's argument typeable against the expression used.
    auto arg_expr_typeable = AnnotateChild(*arg_expr);

    if (!(result = arg_typeable->Unify(arg_expr_typeable))) {
      log_.Fatal(result, node.start);
    }
  }
  out_typeable = call_solver->yield();

  auto call_typeable = Typeable::Create(std::move(call_solver));
  // After constructing a constrained invokee type, unify the declaration's
  // typeable against the caller's typeable.
  if (!(result = func_typeable->Unify(call_typeable))) {
    log_.Fatal(result, node.start);
  }

  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node, TypeablePtr& out_typeable) {
  auto guard_typeable = Typeable::Create();
  for (auto& guard_case : node.cases) {
    auto case_typeable = AnnotateChild(*guard_case.second);
    assert(guard_typeable->Unify(case_typeable));
  }
  auto wildcard_typeable = AnnotateChild(*node.wildcard_case);
  assert(guard_typeable->Unify(wildcard_typeable));

  out_typeable = guard_typeable;

  return false;
}

bool ExpressionTypeTransform::Bind(ast::BindNode& node, TypeablePtr& out_typeable) {
  TypeableScope bind_scope(&scope_);

  // Compute the type of the identifier-bound expression, and use it in the
  // scope of the following body.
  auto expr_typeable = AnnotateChild(*node.expr);
  // TODO(acomminos): throw error if name is already bound?
  bind_scope.Assign(node.identifier, expr_typeable.get());

  auto body_typeable = AnnotateChild(*node.body, bind_scope);

  auto typeable = Typeable::Create();
  assert(typeable->Unify(body_typeable));

  out_typeable = typeable;

  return false;
}

}  // namespace typing
}  // namespace darlang
