#ifndef DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
#define DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_

#include <memory>
#include <unordered_map>
#include "ast/types.h"
#include "ast/util.h"
#include "typing/solver.h"
#include "util/scoped_map.h"
#include "logger.h"

namespace darlang {
namespace typing {

// Mapping of an identifier to a typeable owned by some AST node.
typedef util::ScopedMap<std::string, Typeable*> TypeableScope;
// Mapping of nodes to typeable annotations.
// Owns the memory for all typeables.
typedef std::unordered_map<ast::NodeID, TypeablePtr> TypeableMap;
// The output of the pass, storing materialized types for typeable nodes.
typedef std::unordered_map<ast::NodeID, std::unique_ptr<Type>> TypeMap;

class Specializer;

// Traverses the given AST node, solving for a valid type assignment.
class TypeTransform : public ast::Reducer<TypeMap> {
 public:
  TypeTransform(Logger& log) : log_(log) {}
  bool Module(ast::ModuleNode& node) override;
 private:
  Logger& log_;
};

// Recursively annotates expression nodes with typeables, and returns the
// typeable acting as the return value for the expression.
class ExpressionTypeTransform : public ast::AnnotatedVisitor<TypeablePtr> {
 public:
  ExpressionTypeTransform(Logger& log, TypeableMap& typeables, const TypeableScope& scope, Specializer& specializer)
    : AnnotatedVisitor(typeables), log_(log), scope_(scope), specializer_(specializer) {}

  // Annotates the given node using this transform, and returns the resulting
  // typeable generated.
  TypeablePtr Annotate(ast::Node& node) {
    node.Visit(*this);
    return annotations().at(node.id);
  }

 private:
  // Recursively annotates the given child node, optionally with a modified
  // scope.
  TypeablePtr AnnotateChild(ast::Node& node) {
    return AnnotateChild(node, scope_);
  }
  TypeablePtr AnnotateChild(ast::Node& node, const TypeableScope& scope);

  bool IdExpression(ast::IdExpressionNode& node, TypeablePtr& out_typeable) override;
  bool IntegralLiteral(ast::IntegralLiteralNode& node, TypeablePtr& out_typeable) override;
  bool Invocation(ast::InvocationNode& node, TypeablePtr& out_typeable) override;
  bool Guard(ast::GuardNode& node, TypeablePtr& out_typeable) override;
  bool Bind(ast::BindNode& node, TypeablePtr& out_typeable) override;
  bool Tuple(ast::TupleNode& node, TypeablePtr& out_typeable) override;

  Logger& log_;
  const TypeableScope& scope_;
  Specializer& specializer_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
