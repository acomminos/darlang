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

// Traverses the given AST node, solving for a valid type assignment.
class TypeTransform : public ast::Reducer<TypeMap> {
 public:
  TypeTransform(Logger& log) : log_(log) {}
  bool Module(ast::ModuleNode& node) override;
 private:
  Logger& log_;
};

// Computes typeable constraints from a declaration.
// Writes the traversed declaration to the given map of globals to typeables,
// and recurses on function bodies.
class DeclarationTypeTransform : public ast::Visitor {
 public:
  DeclarationTypeTransform(Logger& log, TypeableMap& typeables, TypeableScope& globals)
    : log_(log), typeables_(typeables), global_scope_(globals) {}

  bool Declaration(ast::DeclarationNode& node) override;

 private:
  Logger& log_;
  TypeableMap& typeables_;
  TypeableScope& global_scope_;
};

// Recursively annotates expression nodes with typeables, and returns the
// typeable acting as the return value for the expression.
class ExpressionTypeTransform : public ast::Annotator<TypeablePtr> {
 public:
  ExpressionTypeTransform(Logger& log, TypeableMap& typeables, const TypeableScope& scope)
    : Annotator(typeables), log_(log), scope_(scope) {}

 private:
  // Recursively annotates the given child node, optionally with a modified
  // scope.
  TypeablePtr AnnotateChild(ast::Node& node) {
    return AnnotateChild(node, scope_);
  }
  TypeablePtr AnnotateChild(ast::Node& node, const TypeableScope& scope);

  bool IdExpression(ast::IdExpressionNode& node) override;
  bool IntegralLiteral(ast::IntegralLiteralNode& node) override;
  bool Invocation(ast::InvocationNode& node) override;
  bool Guard(ast::GuardNode& node) override;
  bool Bind(ast::BindNode& node) override;

  Logger& log_;
  const TypeableScope& scope_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPE_TRANSFORM_H_
