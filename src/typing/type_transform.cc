#include "type_transform.h"

#include <cassert>
#include "typing/function_specializer.h"
#include "typing/disjoint_solver.h"
#include "typing/tuple_solver.h"
#include "typing/primitive_solver.h"
#include "typing/typeable.h"
#include "util/declaration_mapper.h"

namespace darlang {
namespace typing {

TypeablePtr ExpressionTypeTransform::AnnotateChild(ast::Node& node, const TypeableScope& scope) {
  return ExpressionTypeTransform(log_, annotations(), scope, specializer_).Annotate(node);
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

bool ExpressionTypeTransform::StringLiteral(ast::StringLiteralNode& node, TypeablePtr& out_typeable) {
  auto solver = std::make_unique<PrimitiveSolver>(PrimitiveType::String);
  out_typeable = Typeable::Create(std::move(solver));
  return false;
}

bool ExpressionTypeTransform::Invocation(ast::InvocationNode& node, TypeablePtr& out_typeable) {
  // TODO(acomminos): don't perform polymorphic dispatch typing for lambdas
  std::vector<TypeablePtr> args;
  for (int i = 0; i < node.args.size(); i++) {
    args.push_back(AnnotateChild(*node.args[i]));
  }
  auto yield = Typeable::Create();

  Result result;
  if (!(result = specializer_.Specialize(node.callee, args, yield))) {
    log_.Fatal(result, node.start);
  }
  out_typeable = yield;
  return false;
}

bool ExpressionTypeTransform::Guard(ast::GuardNode& node, TypeablePtr& out_typeable) {
  std::vector<TypeablePtr> case_types;
  for (auto& guard_case : node.cases) {
    case_types.push_back(AnnotateChild(*guard_case.second));
  }
  case_types.push_back(AnnotateChild(*node.wildcard_case));

  // Attempt to unify all branches of the guard expression. If this fails, fall
  // back to a disjoint type.
  std::vector<TypeablePtr> reduced_case_types;
  for (auto& type : case_types) {
    // Invariant: all elements of `reduced_case_types` are disjoint.
    bool unified = false;
    for (auto& reduced_type : reduced_case_types) {
      if (type->Unify(reduced_type)) {
        unified = true;
        break;
      }
    }
    if (!unified) {
      reduced_case_types.push_back(type);
    }
  }

  if (reduced_case_types.size() == 1) {
    out_typeable = case_types.front();
  } else {
    auto disjoint_solver = std::make_unique<DisjointSolver>();
    for (auto& type : reduced_case_types) {
      disjoint_solver->Add(type);
    }
    out_typeable = Typeable::Create(std::move(disjoint_solver));
  }

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

bool ExpressionTypeTransform::Tuple(ast::TupleNode& node, TypeablePtr& out_typeable) {
  auto solver = std::make_unique<TupleSolver>(node.items.size());
  auto& items = solver->items();
  for (int i = 0; i < node.items.size(); i++) {
    auto& child_node = std::get<ast::NodePtr>(node.items[i]);
    Result result;

    // Check to make sure the tag at the item's ordinal position does not
    // conflict with any other tag specifiers.
    auto& child_tag = std::get<std::string>(node.items[i]);
    if (!(result = solver->TagItem(i, child_tag))) {
      log_.Fatal(result, child_node->start);
    }

    // Finally, unify the tuple item's type against the expression's type.
    auto& item_typeable = std::get<TypeablePtr>(items[i]);
    if (!(result = item_typeable->Unify(AnnotateChild(*child_node)))) {
      log_.Fatal(result, child_node->start);
    }
  }

  out_typeable = Typeable::Create(std::move(solver));
  return false;
}

}  // namespace typing
}  // namespace darlang
